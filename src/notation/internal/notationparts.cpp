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
#include "engraving/editing/editstaff.h"
#include "engraving/editing/editstavesharing.h"
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
        const std::vector<Part*>& parts = _score->parts();
        result.assign(parts.begin(), parts.end());
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

    const std::vector<Staff*>& staves = part->staves();
    result.assign(staves.begin(), staves.end());

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
    return score()->partById(partId);
}

bool NotationParts::partExists(const ID& partId) const
{
    return part(partId) != nullptr;
}

const Staff* NotationParts::staff(const ID& staffId) const
{
    return score()->staffById(staffId);
}

bool NotationParts::staffExists(const ID& staffId) const
{
    return staff(staffId) != nullptr;
}

StaffConfig NotationParts::staffConfig(const ID& staffId, Fraction tick) const
{
    StaffConfig config;
    const Staff* staff = score()->staffById(staffId);
    if (!staff) {
        return config;
    }

    const mu::engraving::StaffType* staffType = staff->staffType(tick);
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

    transaction(TranslatableString("undoableAction", "Add/remove instruments"), [&](engraving::Transaction&) {
        const EditPart::SetPartsResult result = EditPart::setParts(score(), partEntries(parts), order);

        for (const Part* part : result.addedParts) {
            m_partChangedNotifier.itemAdded(part);
        }

        m_scoreOrderChanged.notify();

        updatePartsAndSystemObjectStaves();
    });
}

void NotationParts::setScoreOrder(const ScoreOrder& order)
{
    if (score()->scoreOrder() == order) {
        return;
    }

    transaction(TranslatableString("undoableAction", "Set score order"), [&](engraving::Transaction&) {
        EditPart::setScoreOrder(score(), order);
    });

    m_scoreOrderChanged.notify();
}

void NotationParts::setPartVisible(const ID& partId, bool visible)
{
    TRACEFUNC;

    Part* part = score()->partById(partId);
    if (!part) {
        return;
    }

    if (part->show() == visible) {
        return;
    }

    const TranslatableString actionName = visible
                                          ? TranslatableString("undoableAction", "Show instrument")
                                          : TranslatableString("undoableAction", "Hide instrument");

    transaction(actionName, [&](engraving::Transaction&) {
        EditPart::setPartVisible(score(), part, visible);
    });

    notifyAboutPartChanged(part);
}

void NotationParts::setPartSharpFlat(const ID& partId, const SharpFlat& sharpFlat)
{
    TRACEFUNC;

    Part* part = score()->partById(partId);
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

    transaction(calcActionName(sharpFlat), [&](engraving::Transaction&) {
        EditPart::setPartSharpFlat(score(), part, sharpFlat);
    });

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

    Part* part = score()->partById(instrumentKey.partId);
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

    transaction(TranslatableString("undoableAction", "Set instrument name"), [&](engraving::Transaction&) {
        EditPart::setInstrumentName(score(), part, instrumentKey.tick, name);
    });

    notifyAboutPartChanged(part);
}

void NotationParts::setInstrumentAbbreviature(const InstrumentKey& instrumentKey, const QString& abbreviature)
{
    TRACEFUNC;

    Part* part = score()->partById(instrumentKey.partId);
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

    transaction(TranslatableString("undoableAction", "Set abbreviated instrument name"), [&](engraving::Transaction&) {
        EditPart::setInstrumentAbbreviature(score(), part, instrumentKey.tick, abbreviature);
    });

    notifyAboutPartChanged(part);
}

void NotationParts::setInstrumentGroupNameOptions(const std::vector<InstrumentKey>& instruments, bool useCustom, const QString& name,
                                                  const QString& shortName)
{
    TRACEFUNC;

    std::vector<Part*> changedParts;

    transaction(TranslatableString("undoableAction", "Set instrument custom group name"), [&](engraving::Transaction&) {
        for (const InstrumentKey& key : instruments) {
            Part* part = score()->partById(key.partId);
            if (EditPart::setInstrumentGroupNameOptions(score(), part, key.tick, useCustom, name, shortName)) {
                changedParts.push_back(part);
            }
        }
    });

    for (Part* p : changedParts) {
        notifyAboutPartChanged(p);
    }
}

void NotationParts::setInstrumentNumber(const InstrumentKey& instrumentKey, int v)
{
    TRACEFUNC;

    Part* part = score()->partById(instrumentKey.partId);
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

    transaction(TranslatableString("undoableAction", "Set instrument number"), [&](engraving::Transaction&) {
        EditPart::setInstrumentNumber(score(), part, instrumentKey.tick, v);
    });

    notifyAboutPartChanged(part);
}

bool NotationParts::setVoiceVisible(const ID& staffId, int voiceIndex, bool visible)
{
    TRACEFUNC;

    Staff* staff = score()->staffById(staffId);
    if (!staff) {
        return false;
    }

    const TranslatableString actionName = visible
                                          ? TranslatableString("undoableAction", "Show voice %1").arg(voiceIndex + 1)
                                          : TranslatableString("undoableAction", "Hide voice %1").arg(voiceIndex + 1);

    bool result = false;

    //! HACK: Excerpt::setVoiceVisible recreates the staff,
    //! so later in listenUndoStackChanges() we will call notifyAboutStaffRemoved() and notifyAboutStaffAdded(),
    //! which will result in the wrong UI state in the Layout panel.
    //! We should not recreate the staff here, only update it
    m_ignoreUndoStackChanges = true;
    transaction(actionName, [&](engraving::Transaction&) {
        result = EditPart::setVoiceVisible(score(), staff, voiceIndex, visible);
    });
    m_ignoreUndoStackChanges = false;

    if (result) {
        notifyAboutStaffChanged(score()->staffById(staffId));
    }

    return result;
}

void NotationParts::setStaffVisible(const ID& staffId, bool visible)
{
    TRACEFUNC;

    Staff* staff = score()->staffById(staffId);
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

    config.visible = visible;

    transaction(actionName, [&](engraving::Transaction&) {
        EditPart::setStaffConfig(score(), staff, config);
    });

    notifyAboutStaffChanged(staff);
}

void NotationParts::setStaffType(const ID& staffId, StaffTypeId type)
{
    TRACEFUNC;

    Staff* staff = score()->staffById(staffId);
    const mu::engraving::StaffType* staffType = mu::engraving::StaffType::preset(type);

    if (!staff || !staffType) {
        return;
    }

    if (staff->staffType(DEFAULT_TICK) == staffType) {
        return;
    }

    transaction(TranslatableString("undoableAction", "Set staff type"), [&](engraving::Transaction&) {
        EditPart::setStaffType(score(), staff, type);
    });

    notifyAboutStaffChanged(staff);
}

void NotationParts::setStaffConfig(const ID& staffId, const StaffConfig& config, Fraction tick)
{
    TRACEFUNC;

    Staff* staff = score()->staffById(staffId);
    if (!staff) {
        return;
    }

    if (staffConfig(staffId) == config) {
        return;
    }

    transaction(TranslatableString("undoableAction", "Edit staff properties"), [&](engraving::Transaction&) {
        EditPart::setStaffConfig(score(), staff, config, tick);
    });

    notifyAboutStaffChanged(staff);
}

void NotationParts::setSharedPartEnabled(const muse::ID& partId, bool enable)
{
    TRACEFUNC;

    Part* part = score()->partById(partId);
    if (!part) {
        return;
    }

    if (part->getProperty(Pid::SHARED_PART_ENABLED).toBool() == enable) {
        return;
    }

    const TranslatableString actionName = enable
                                          ? TranslatableString("undoableAction", "Enable shared staff")
                                          : TranslatableString("undoableAction", "Disable shared staff");

    transaction(actionName, [&](engraving::Transaction&) {
        EditPart::setSharedPartEnabled(score(), part, enable);
    });

    notifyAboutPartChanged(part);
}

bool NotationParts::appendStaff(Staff* staff, const ID& destinationPartId)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(staff) {
        return false;
    }

    Part* destinationPart = score()->partById(destinationPartId);
    if (!destinationPart) {
        return false;
    }

    transaction(TranslatableString("undoableAction", "Add staff"), [&](engraving::Transaction&) {
        EditPart::appendStaff(score(), staff, destinationPart);
    });

    return true;
}

bool NotationParts::appendLinkedStaff(Staff* staff, const muse::ID& sourceStaffId, const muse::ID& destinationPartId)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(staff) {
        return false;
    }

    Staff* sourceStaff = score()->staffById(sourceStaffId);
    Part* destinationPart = score()->partById(destinationPartId);
    if (!sourceStaff || !destinationPart) {
        return false;
    }

    transaction(TranslatableString("undoableAction", "Add linked staff"), [&](engraving::Transaction&) {
        EditPart::appendLinkedStaff(score(), staff, sourceStaff, destinationPart);
    });

    return true;
}

void NotationParts::insertPart(Part* part, size_t index)
{
    TRACEFUNC;

    if (!part) {
        return;
    }

    transaction(TranslatableString("undoableAction", "Add instrument"), [&](engraving::Transaction&) {
        EditPart::insertPart(score(), part, index);
    });

    notifyAboutPartAdded(part);
}

void NotationParts::replacePart(const ID& partId, Part* newPart)
{
    TRACEFUNC;

    Part* part = score()->partById(partId);
    if (!part || !newPart) {
        return;
    }

    transaction(TranslatableString("undoableAction", "Reset instrument formatting"), [&](engraving::Transaction&) {
        EditPart::replacePart(score(), part, newPart);
    });

    notifyAboutPartReplaced(part, newPart);
}

void NotationParts::replaceInstrument(const InstrumentKey& instrumentKey, const Instrument& newInstrument, const StaffType* newStaffType)
{
    TRACEFUNC;

    Part* part = score()->partById(instrumentKey.partId);
    if (!part) {
        return;
    }

    bool replaced = false;

    transaction(TranslatableString("undoableAction", "Replace instrument"), [&](engraving::Transaction&) {
        replaced = EditPart::replaceInstrument(score(), part, instrumentKey.tick, newInstrument, newStaffType);
    });

    if (replaced) {
        notifyAboutPartChanged(part);
    }
}

void NotationParts::replaceDrumset(const InstrumentKey& instrumentKey, const Drumset& newDrumset, bool undoable)
{
    Part* part = score()->partById(instrumentKey.partId);
    if (!part) {
        return;
    }

    auto replace = [this, part, &instrumentKey, &newDrumset, undoable]() {
        for (const auto& pair : part->instruments()) {
            const Instrument* instrument = pair.second;
            if (instrument && instrument->drumset() && instrument->id() == instrumentKey.instrumentId) {
                EditPart::replaceDrumset(score(), part, Fraction::fromTicks(pair.first), newDrumset, undoable);
            }
        }
    };

    if (undoable) {
        transaction(TranslatableString("undoableAction", "Edit drumset"), [&replace](engraving::Transaction&) {
            replace();
        });
    } else {
        replace();
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

    transaction(TranslatableString("undoableAction", "Add system markings"), [&](engraving::Transaction&) {
        EditPart::addSystemObjects(score(), staves);
    });
}

void NotationParts::removeSystemObjects(const IDList& stavesIds)
{
    std::vector<Staff*> staves = this->staves(stavesIds);
    if (staves.empty()) {
        return;
    }

    transaction(TranslatableString("undoableAction", "Remove system markings"), [&](engraving::Transaction&) {
        EditPart::removeSystemObjects(score(), staves);
    });
}

void NotationParts::toggleStaveSharing(bool on)
{
    transaction(TranslatableString("undoableAction", "Toggle stave sharing"), [&](engraving::Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score(), on);
    });
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

void NotationParts::transaction(const TranslatableString& actionName, const std::function<void(engraving::Transaction&)>& func)
{
    if (undoStack()->isLocked()) {
        func(score()->transactionManager()->currentOrDummyTransaction());
    } else {
        undoStack()->transaction(actionName, func);
    }

    m_partsChanged.notify();
}

void NotationParts::removeParts(const IDList& partsIds)
{
    TRACEFUNC;

    std::vector<Part*> partsToRemove = parts(partsIds);
    if (partsToRemove.empty()) {
        return;
    }

    endInteractionWithScore();
    transaction(TranslatableString("undoableAction", "Remove instruments"), [&](engraving::Transaction&) {
        EditPart::removeParts(score(), partsToRemove);
    });

    for (const Part* part : partsToRemove) {
        notifyAboutPartRemoved(part);
    }
}

void NotationParts::removeStaves(const IDList& stavesIds)
{
    TRACEFUNC;

    std::vector<Staff*> stavesToRemove = staves(stavesIds);
    if (stavesToRemove.empty()) {
        return;
    }

    endInteractionWithScore();
    transaction(TranslatableString("undoableAction", "Remove staves"), [&](engraving::Transaction&) {
        EditPart::removeStaves(score(), stavesToRemove);
    });
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
    transaction(TranslatableString("undoableAction", "Move instruments"), [&](engraving::Transaction&) {
        EditPart::reorderParts(score(), orderedParts, moves);
    });
}

std::vector<EditPart::SystemObjectsMove> NotationParts::editPartSystemObjectsMoves(const std::vector<SystemObjectsMove>& moves) const
{
    std::vector<EditPart::SystemObjectsMove> result;
    result.reserve(moves.size());

    for (const SystemObjectsMove& move : moves) {
        result.push_back({ score()->staffById(move.staffId), score()->staffById(move.destinationStaffId), move.belowBottomStaff });
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
    transaction(TranslatableString("undoableAction", "Move staves"), [&](engraving::Transaction&) {
        EditPart::reorderStaves(score(), orderedStaves);
    });
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
