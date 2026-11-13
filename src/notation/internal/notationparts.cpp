/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "notationparts.h"

#include "translation.h"

#include "engraving/dom/instrument.h"
#include "engraving/editing/editpagelocks.h"
#include "engraving/editing/editstaff.h"
#include "engraving/editing/editstavesharing.h"
#include "engraving/editing/editsystemlocks.h"
#include "engraving/editing/transaction/transaction.h"

#include "igetscore.h"
#include "inotationnoteinput.h" // IWYU pragma: keep

#include "log.h"

using namespace muse;
using namespace muse::async;
using namespace mu::notation;
using namespace mu::engraving;

static const mu::engraving::Fraction DEFAULT_TICK = mu::engraving::Fraction(0, 1);

NotationParts::NotationParts(IGetScore* getScore, INotationInteractionPtr interaction, INotationUndoStackPtr undoStack,
                             INotationStylePtr style)
    : m_getScore(getScore), m_undoStack(undoStack), m_interaction(interaction), m_style(style)
{
    m_getScore->scoreInited().onNotify(this, [this]() {
        listenUndoStackChanges();
        listenStyleChanges();
    });
}

NotifyList<const Part*> NotationParts::partList() const
{
    NotifyList<const Part*> result;
    result.setNotify(m_partChangedNotifier.notify());

    if (Score* _score = score()) {
        for (const Part* part: _score->parts()) {
            result.push_back(part);
        }
    }

    return result;
}

NotifyList<const Staff*> NotationParts::staffList(const ID& partId) const
{
    NotifyList<const Staff*> result;
    ChangedNotifier<const Staff*>& notifier = m_staffChangedNotifierMap[partId];
    result.setNotify(notifier.notify());

    const Part* part = this->part(partId);
    if (!part) {
        return result;
    }

    for (const Staff* staff: part->staves()) {
        result.push_back(staff);
    }

    return result;
}

bool NotationParts::hasParts() const
{
    const Score* score = this->score();
    if (!score) {
        return false;
    }

    return !score->parts().empty();
}

const Part* NotationParts::part(const ID& partId) const
{
    return partModifiable(partId);
}

bool NotationParts::partExists(const ID& partId) const
{
    return part(partId) != nullptr;
}

const Staff* NotationParts::staff(const ID& staffId) const
{
    return staffModifiable(staffId);
}

bool NotationParts::staffExists(const ID& staffId) const
{
    return staff(staffId) != nullptr;
}

StaffConfig NotationParts::staffConfig(const ID& staffId, Fraction tick) const
{
    StaffConfig config;
    Staff* staff = staffModifiable(staffId);
    if (!staff) {
        return config;
    }

    mu::engraving::StaffType* staffType = staff->staffType(tick);
    if (!staffType) {
        return config;
    }

    config.visible = staff->visible();
    config.userDistance = staff->userDist();
    config.cutaway = staff->cutaway();
    config.hideSystemBarline = staff->hideSystemBarLine();
    config.mergeMatchingRests = staff->mergeMatchingRests();
    config.clefTypeList = staff->defaultClefType();
    config.reflectTranspositionInLinkedTab = staff->reflectTranspositionInLinkedTab();

    config.staffType = *staffType;

    return config;
}

ScoreOrder NotationParts::scoreOrder() const
{
    return score()->scoreOrder();
}

Part* NotationParts::partModifiable(const ID& partId) const
{
    return score()->partById(partId.toUint64());
}

Staff* NotationParts::staffModifiable(const ID& staffId) const
{
    return score()->staffById(staffId.toUint64());
}

std::vector<Staff*> NotationParts::staves(const IDList& stavesIds) const
{
    std::vector<Staff*> staves;
    staves.reserve(stavesIds.size());

    for (const ID& staffId : stavesIds) {
        if (Staff* staff = score()->staffById(staffId)) {
            staves.push_back(staff);
        }
    }

    return staves;
}

std::vector<Part*> NotationParts::parts(const IDList& partsIds) const
{
    std::vector<Part*> parts;
    parts.reserve(partsIds.size());

    for (const ID& partId : partsIds) {
        if (Part* part = score()->partById(partId)) {
            parts.push_back(part);
        }
    }

    return parts;
}

void NotationParts::setParts(const PartInstrumentList& parts, const ScoreOrder& order)
{
    TRACEFUNC;

    endInteractionWithScore();
    startEdit(TranslatableString("undoableAction", "Add/remove instruments"));

    doSetParts(parts, order);
    updatePartsAndSystemObjectStaves();

    apply();
}

void NotationParts::setScoreOrder(const ScoreOrder& order)
{
    if (score()->scoreOrder() == order) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Set score order"));

    EditPart::setScoreOrder(score(), order);
    m_scoreOrderChanged.notify();

    apply();
}

void NotationParts::setPartVisible(const ID& partId, bool visible)
{
    TRACEFUNC;

    Part* part = partModifiable(partId);
    if (!part) {
        return;
    }

    if (part->show() == visible) {
        return;
    }

    const TranslatableString actionName = visible
                                          ? TranslatableString("undoableAction", "Show instrument")
                                          : TranslatableString("undoableAction", "Hide instrument");

    startEdit(actionName);

    mu::engraving::EditPart::setPartVisible(score(), part, visible);

    apply();

    notifyAboutPartChanged(part);
}

void NotationParts::setPartSharpFlat(const ID& partId, const SharpFlat& sharpFlat)
{
    TRACEFUNC;

    Part* part = partModifiable(partId);
    if (!part) {
        return;
    }

    int shartFlatInt = static_cast<int>(sharpFlat);
    if (part->getProperty(mu::engraving::Pid::PREFER_SHARP_FLAT) == shartFlatInt) {
        return;
    }

    auto calcActionName = [](const SharpFlat& sharpFlat) -> TranslatableString {
        switch (sharpFlat) {
        case SharpFlat::NONE: return TranslatableString("undoableAction", "Set sharps/flats no preference");
        case SharpFlat::FLATS: return TranslatableString("undoableAction", "Set prefer flats");
        case SharpFlat::SHARPS: return TranslatableString("undoableAction", "Set prefer sharps");
        case SharpFlat::AUTO: return TranslatableString("undoableAction", "Set sharps/flats automatic");
        }
        return TranslatableString("undoableAction", "Set sharps/flats preference");
    };

    startEdit(calcActionName(sharpFlat));

    mu::engraving::EditPart::setPartSharpFlat(score(), part, sharpFlat);

    apply();

    notifyAboutPartChanged(part);
}

void NotationParts::listenUndoStackChanges()
{
    if (!score()) {
        return;
    }

    updatePartsAndSystemObjectStaves();

    m_undoStack->changesChannel().onReceive(this, [this](const ScoreChanges& changes) {
        if (changes.isTextEditing || changes.changedTypes.empty() || m_ignoreUndoStackChanges) {
            return;
        }

        static const ElementTypeSet TYPES_TO_CHECK {
            ElementType::SCORE,
            ElementType::STAFF,
            ElementType::PART,
            ElementType::SHARED_PART,
        };

        for (ElementType type : TYPES_TO_CHECK) {
            if (muse::contains(changes.changedTypes, type)) {
                updatePartsAndSystemObjectStaves(changes);
                return;
            }
        }
    });
}

void NotationParts::updatePartsAndSystemObjectStaves(const mu::engraving::ScoreChanges& changes)
{
    const bool partsChanged = m_parts != score()->parts();
    m_parts = score()->parts();

    std::vector<Staff*> newSystemObjectStaves = score()->systemObjectStavesWithTopStaff();
    const bool systemObjectStavesChanged = m_systemObjectStaves != newSystemObjectStaves;
    m_systemObjectStaves = std::move(newSystemObjectStaves);

    if (partsChanged) {
        m_partChangedNotifier.changed();
    }

    if (systemObjectStavesChanged) {
        m_systemObjectStavesChanged.notify();
    }

    if (muse::contains(changes.changedTypes, ElementType::SHARED_PART)) {
        m_sharedPartsChanged.notify();
    }

    std::vector<Staff*> removedStaves;
    std::vector<Staff*> addedStaves;

    bool stavesSorted = false;

    for (auto& pair : changes.changedObjects) {
        if (muse::contains(pair.second, CommandType::SortStaves)) {
            stavesSorted = true;
            break;
        }

        if (!pair.first->isStaff()) {
            continue;
        }

        Staff* staff = toStaff(pair.first);

        if (muse::contains(pair.second, CommandType::RemoveStaff)) {
            removedStaves.push_back(staff);
        } else if (muse::contains(pair.second, CommandType::InsertStaff)) {
            addedStaves.push_back(staff);
        }
    }

    if (stavesSorted && !partsChanged) {
        m_partChangedNotifier.changed();
        return;
    }

    for (Staff* staff : removedStaves) {
        notifyAboutStaffRemoved(staff);
    }

    for (Staff* staff: addedStaves) {
        notifyAboutStaffAdded(staff);
    }
}

void NotationParts::listenStyleChanges()
{
    if (!score()) {
        return;
    }

    m_style->styleChanged().onNotify(this, [this]{ m_sharedPartsChanged.notify(); });
}

EditPart::SetPartsResult NotationParts::doSetParts(const PartInstrumentList& parts, const ScoreOrder& order)
{
    EditPart::SetPartsResult result = EditPart::setParts(score(), partEntries(parts), order);

    for (Part* part : result.addedParts) {
        m_partChangedNotifier.itemAdded(part);
    }

    m_scoreOrderChanged.notify();

    return result;
}

std::vector<EditPart::PartEntry> NotationParts::partEntries(const PartInstrumentList& parts) const
{
    std::vector<EditPart::PartEntry> entries;
    entries.reserve(parts.size());

    for (const PartInstrument& pi : parts) {
        if (pi.isExistingPart) {
            entries.push_back({ score()->partById(pi.partId), nullptr, pi.isSoloist });
        } else {
            entries.push_back({ nullptr, &pi.instrumentTemplate, pi.isSoloist });
        }
    }

    return entries;
}

void NotationParts::setInstrumentName(const InstrumentKey& instrumentKey, const QString& name)
{
    TRACEFUNC;

    Part* part = partModifiable(instrumentKey.partId);
    if (!part) {
        return;
    }

    const mu::engraving::Instrument* instrument = part->instrument(instrumentKey.tick);
    if (!instrument) {
        return;
    }

    String newName = String::fromQString(name);
    if (instrument->longName() == newName) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Set instrument name"));

    mu::engraving::EditPart::setInstrumentName(score(), part, instrumentKey.tick, name);

    apply();

    notifyAboutPartChanged(part);
}

void NotationParts::setInstrumentAbbreviature(const InstrumentKey& instrumentKey, const QString& abbreviature)
{
    TRACEFUNC;

    Part* part = partModifiable(instrumentKey.partId);
    if (!part) {
        return;
    }

    const mu::engraving::Instrument* instrument = part->instrument(instrumentKey.tick);
    if (!instrument) {
        return;
    }

    if (instrument->abbreviatureAsPlainText() == abbreviature) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Set abbreviated instrument name"));

    mu::engraving::EditPart::setInstrumentAbbreviature(score(), part, instrumentKey.tick, abbreviature);

    apply();

    notifyAboutPartChanged(part);
}

void NotationParts::setInstrumentGroupNameOptions(const std::vector<InstrumentKey>& instruments, bool useCustom, const QString& name,
                                                  const QString& shortName)
{
    TRACEFUNC;

    startEdit(TranslatableString("undoableAction", "Set instrument custom group name"));

    std::vector<Part*> changedParts;

    for (const InstrumentKey& key : instruments) {
        Part* part = partModifiable(key.partId);
        if (!part) {
            continue;
        }

        const mu::engraving::Instrument* instrument = part->instrument(key.tick);
        if (!instrument) {
            continue;
        }

        const InstrumentLabel& instrLabel = instrument->instrumentLabel();
        if (instrLabel.useCustomGroupName() == useCustom && instrLabel.customNameLongGroup() == name
            && instrLabel.customNameShortGroup() == shortName) {
            continue;
        }

        mu::engraving::EditPart::setInstrumentGroupNameOptions(score(), part, key.tick, useCustom, name, shortName);

        changedParts.push_back(part);
    }

    apply();

    for (Part* p : changedParts) {
        notifyAboutPartChanged(p);
    }
}

void NotationParts::setInstrumentNumber(const InstrumentKey& instrumentKey, int v)
{
    TRACEFUNC;

    Part* part = partModifiable(instrumentKey.partId);
    if (!part) {
        return;
    }

    const mu::engraving::Instrument* instrument = part->instrument(instrumentKey.tick);
    if (!instrument) {
        return;
    }

    if (instrument->number() == v) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Set instrument number"));

    EditPart::setInstrumentNumber(score(), part, instrumentKey.tick, v);

    apply();

    notifyAboutPartChanged(part);
}

bool NotationParts::setVoiceVisible(const ID& staffId, int voiceIndex, bool visible)
{
    TRACEFUNC;

    Staff* staff = staffModifiable(staffId);
    if (!staff) {
        return false;
    }

    const TranslatableString actionName = visible
                                          ? TranslatableString("undoableAction", "Show voice %1").arg(voiceIndex + 1)
                                          : TranslatableString("undoableAction", "Hide voice %1").arg(voiceIndex + 1);

    startEdit(actionName);

    bool result = EditPart::setVoiceVisible(score(), staff, voiceIndex, visible);
    if (!result) {
        rollback();
        return false;
    }

    //! HACK: Excerpt::setVoiceVisible recreates the staff,
    //! so later in listenUndoStackChanges() we will call notifyAboutStaffRemoved() and notifyAboutStaffAdded(),
    //! which will result in the wrong UI state in the Layout panel.
    //! We should not recreate the staff here, only update it
    m_ignoreUndoStackChanges = true;
    apply();
    m_ignoreUndoStackChanges = false;

    Staff* newStaff = staffModifiable(staffId);
    notifyAboutStaffChanged(newStaff);

    return true;
}

void NotationParts::setStaffVisible(const ID& staffId, bool visible)
{
    TRACEFUNC;

    Staff* staff = staffModifiable(staffId);
    if (!staff) {
        return;
    }

    StaffConfig config = staffConfig(staffId);
    if (config.visible == visible) {
        return;
    }

    const TranslatableString actionName = visible
                                          ? TranslatableString("undoableAction", "Show staff")
                                          : TranslatableString("undoableAction", "Hide staff");

    startEdit(actionName);

    config.visible = visible;
    doSetStaffConfig(staff, config);

    if (visible) {
        engraving::Transaction& tx = score()->transactionManager()->currentOrDummyTransaction();
        EditPageLocks::removePageLocksContainingMMRests(tx, score());
        EditSystemLocks::removeSystemLocksContainingMMRests(tx, score());
    }

    apply();

    notifyAboutStaffChanged(staff);
}

void NotationParts::setStaffType(const ID& staffId, StaffTypeId type)
{
    TRACEFUNC;

    Staff* staff = staffModifiable(staffId);
    const mu::engraving::StaffType* staffType = mu::engraving::StaffType::preset(type);

    if (!staff || !staffType) {
        return;
    }

    if (staff->staffType(DEFAULT_TICK) == staffType) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Set staff type"));

    mu::engraving::EditPart::setStaffType(score(), staff, type);

    apply();

    notifyAboutStaffChanged(staff);
}

void NotationParts::setStaffConfig(const ID& staffId, const StaffConfig& config, Fraction tick)
{
    TRACEFUNC;

    Staff* staff = staffModifiable(staffId);
    if (!staff) {
        return;
    }

    if (staffConfig(staffId) == config) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Edit staff properties"));

    doSetStaffConfig(staff, config, tick);

    apply();

    notifyAboutStaffChanged(staff);
}

void NotationParts::setSharedPartEnabled(const muse::ID& partId, bool enable)
{
    TRACEFUNC;

    Part* part = partModifiable(partId);
    if (!part) {
        return;
    }

    if (part->getProperty(Pid::SHARED_PART_ENABLED).toBool() == enable) {
        return;
    }

    const TranslatableString actionName = enable
                                          ? TranslatableString("undoableAction", "Enable shared staff")
                                          : TranslatableString("undoableAction", "Disable shared staff");

    startEdit(actionName);

    EditPart::setSharedPartEnabled(score(), part, enable);

    apply();

    notifyAboutPartChanged(part);
}

bool NotationParts::appendStaff(Staff* staff, const ID& destinationPartId)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(staff) {
        return false;
    }

    Part* destinationPart = partModifiable(destinationPartId);
    if (!destinationPart) {
        return false;
    }

    startEdit(TranslatableString("undoableAction", "Add staff"));

    EditPart::appendStaff(score(), staff, destinationPart);

    apply();

    return true;
}

bool NotationParts::appendStaffLinkedToMaster(Staff* staff, Staff* masterSourceStaff, const muse::ID& destinationPartId)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(staff && masterSourceStaff) {
        return false;
    }

    Part* destinationPart = partModifiable(destinationPartId);
    if (!destinationPart) {
        return false;
    }

    startEdit(TranslatableString("undoableAction", "Add staff"));

    EditPart::appendStaffLinkedToMaster(score(), staff, masterSourceStaff, destinationPart);

    apply();

    return true;
}

bool NotationParts::appendLinkedStaff(Staff* staff, const muse::ID& sourceStaffId, const muse::ID& destinationPartId)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(staff) {
        return false;
    }

    Staff* sourceStaff = staffModifiable(sourceStaffId);
    Part* destinationPart = partModifiable(destinationPartId);
    if (!sourceStaff || !destinationPart) {
        return false;
    }

    startEdit(TranslatableString("undoableAction", "Add linked staff"));

    EditPart::appendLinkedStaff(score(), staff, sourceStaff, destinationPart);

    apply();

    return true;
}

void NotationParts::insertPart(Part* part, size_t index)
{
    TRACEFUNC;

    if (!part) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Add instrument"));

    EditPart::insertPart(score(), part, index);

    apply();

    notifyAboutPartAdded(part);
}

void NotationParts::replacePart(const ID& partId, Part* newPart)
{
    TRACEFUNC;

    Part* part = partModifiable(partId);
    if (!part || !newPart) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Reset instrument formatting"));

    EditPart::replacePart(score(), part, newPart);

    apply();

    notifyAboutPartReplaced(part, newPart);
}

void NotationParts::replaceInstrument(const InstrumentKey& instrumentKey, const Instrument& newInstrument, const StaffType* newStaffType)
{
    TRACEFUNC;

    Part* part = partModifiable(instrumentKey.partId);
    if (!part) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Replace instrument"));

    if (!mu::engraving::EditPart::replaceInstrument(score(), part, instrumentKey.tick, newInstrument, newStaffType)) {
        rollback();
        return;
    }

    apply();

    notifyAboutPartChanged(part);
}

void NotationParts::replaceDrumset(const InstrumentKey& instrumentKey, const Drumset& newDrumset, bool undoable)
{
    Part* part = partModifiable(instrumentKey.partId);
    if (!part) {
        return;
    }

    if (undoable) {
        startEdit(TranslatableString("undoableAction", "Edit drumset"));

        for (auto pair : part->instruments()) {
            Instrument* instrument = pair.second;
            if (instrument && instrument->drumset() && instrument->id() == instrumentKey.instrumentId) {
                EditPart::replaceDrumset(score(), part, Fraction::fromTicks(pair.first), newDrumset);
            }
        }

        apply();
    } else {
        for (auto pair : part->instruments()) {
            Instrument* instrument = pair.second;
            if (instrument && instrument->drumset() && instrument->id() == instrumentKey.instrumentId) {
                instrument->setDrumset(&newDrumset);
            }
        }
    }

    notifyAboutPartChanged(part);
    m_interaction->noteInput()->stateChanged().notify();
}

const std::vector<Staff*>& NotationParts::systemObjectStaves() const
{
    return m_systemObjectStaves;
}

muse::async::Notification NotationParts::systemObjectStavesChanged() const
{
    return m_systemObjectStavesChanged;
}

void NotationParts::addSystemObjects(const muse::IDList& stavesIds)
{
    std::vector<Staff*> staves = this->staves(stavesIds);
    if (staves.empty()) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Add system markings"));

    EditPart::addSystemObjects(score(), staves);

    apply();
}

void NotationParts::removeSystemObjects(const IDList& stavesIds)
{
    std::vector<Staff*> staves = this->staves(stavesIds);
    if (staves.empty()) {
        return;
    }

    startEdit(TranslatableString("undoableAction", "Remove system markings"));

    EditPart::removeSystemObjects(score(), staves);

    apply();
}

void NotationParts::toggleStaveSharing(bool on)
{
    undoStack()->transaction(TranslatableString("undoableAction", "Toggle stave sharing"), [&](engraving::Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score(), on);
    });
    m_partsChanged.notify();
}

Notification NotationParts::sharedPartsChanged() const
{
    return m_sharedPartsChanged;
}

Notification NotationParts::partsChanged() const
{
    return m_partsChanged;
}

Notification NotationParts::scoreOrderChanged() const
{
    return m_scoreOrderChanged;
}

mu::engraving::Score* NotationParts::score() const
{
    return m_getScore->score();
}

INotationUndoStackPtr NotationParts::undoStack() const
{
    return m_undoStack;
}

void NotationParts::startEdit(const muse::TranslatableString& actionName)
{
    undoStack()->prepareChanges(actionName);
}

void NotationParts::apply()
{
    undoStack()->commitChanges();

    m_partsChanged.notify();
}

void NotationParts::rollback()
{
    undoStack()->rollbackChanges();
}

void NotationParts::removeParts(const IDList& partsIds)
{
    TRACEFUNC;

    std::vector<Part*> partsToRemove = parts(partsIds);
    if (partsToRemove.empty()) {
        return;
    }

    endInteractionWithScore();
    startEdit(TranslatableString("undoableAction", "Remove instruments"));

    EditPart::removeParts(score(), partsToRemove);

    apply();

    for (const Part* part : partsToRemove) {
        notifyAboutPartRemoved(part);
    }
}

void NotationParts::doSetStaffConfig(Staff* staff, const StaffConfig& config, Fraction tick)
{
    mu::engraving::StaffType* staffType = staff->staffType(tick);
    if (!staffType) {
        return;
    }

    score()->undo(new mu::engraving::ChangeStaff(staff, config.visible, config.clefTypeList, config.userDistance, config.cutaway,
                                                 config.hideSystemBarline, config.mergeMatchingRests,
                                                 config.reflectTranspositionInLinkedTab));

    score()->undo(new mu::engraving::ChangeStaffType(staff, config.staffType, tick));
}

void NotationParts::removeStaves(const IDList& stavesIds)
{
    TRACEFUNC;

    std::vector<Staff*> stavesToRemove = staves(stavesIds);
    if (stavesToRemove.empty()) {
        return;
    }

    endInteractionWithScore();
    startEdit(TranslatableString("undoableAction", "Remove staves"));

    EditPart::removeStaves(score(), stavesToRemove);

    apply();
}

void NotationParts::reorderParts(const IDList& partIds, const std::vector<SystemObjectsMove>& systemObjectsMoves)
{
    TRACEFUNC;

    const std::vector<Part*> orderedParts = parts(partIds);
    const std::vector<EditPart::SystemObjectsMove> moves = editPartSystemObjectsMoves(systemObjectsMoves);
    if (!EditPart::reorderPartsChangesScore(score(), orderedParts, moves)) {
        return;
    }

    endInteractionWithScore();
    startEdit(TranslatableString("undoableAction", "Move instruments"));

    EditPart::reorderParts(score(), orderedParts, moves);

    apply();
}

std::vector<EditPart::SystemObjectsMove> NotationParts::editPartSystemObjectsMoves(const std::vector<SystemObjectsMove>& moves) const
{
    std::vector<EditPart::SystemObjectsMove> result;
    result.reserve(moves.size());

    for (const SystemObjectsMove& move : moves) {
        result.push_back({ staffModifiable(move.staffId), staffModifiable(move.destinationStaffId), move.belowBottomStaff });
    }

    return result;
}

void NotationParts::reorderStaves(const IDList& staffIds)
{
    TRACEFUNC;

    const std::vector<Staff*> orderedStaves = staves(staffIds);
    if (!EditPart::reorderStavesChangesScore(orderedStaves)) {
        return;
    }

    endInteractionWithScore();
    startEdit(TranslatableString("undoableAction", "Move staves"));

    EditPart::reorderStaves(score(), orderedStaves);

    apply();
}

void NotationParts::endInteractionWithScore()
{
    m_interaction->clearSelection();
    m_interaction->noteInput()->endNoteInput(/*resetState=*/ true);
}

void NotationParts::notifyAboutPartChanged(const Part* part) const
{
    IF_ASSERT_FAILED(part) {
        return;
    }

    m_partChangedNotifier.itemChanged(part);
}

void NotationParts::notifyAboutPartAdded(const Part* part) const
{
    IF_ASSERT_FAILED(part) {
        return;
    }

    m_partChangedNotifier.itemAdded(part);
}

void NotationParts::notifyAboutPartRemoved(const Part* part) const
{
    IF_ASSERT_FAILED(part) {
        return;
    }

    m_partChangedNotifier.itemRemoved(part);
}

void NotationParts::notifyAboutPartReplaced(const Part* oldPart, const Part* newPart) const
{
    IF_ASSERT_FAILED(oldPart && newPart) {
        return;
    }

    m_partChangedNotifier.itemReplaced(oldPart, newPart);
}

void NotationParts::notifyAboutStaffChanged(const Staff* staff) const
{
    IF_ASSERT_FAILED(staff && staff->part()) {
        return;
    }

    ChangedNotifier<const Staff*>& notifier = m_staffChangedNotifierMap[staff->part()->id()];
    notifier.itemChanged(staff);
}

void NotationParts::notifyAboutStaffAdded(const Staff* staff) const
{
    IF_ASSERT_FAILED(staff && staff->part()) {
        return;
    }

    ChangedNotifier<const Staff*>& notifier = m_staffChangedNotifierMap[staff->part()->id()];
    notifier.itemAdded(staff);
}

void NotationParts::notifyAboutStaffRemoved(const Staff* staff) const
{
    IF_ASSERT_FAILED(staff && staff->part()) {
        return;
    }

    ChangedNotifier<const Staff*>& notifier = m_staffChangedNotifierMap[staff->part()->id()];
    notifier.itemRemoved(staff);
}
