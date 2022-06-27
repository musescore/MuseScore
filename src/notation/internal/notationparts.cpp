/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "libmscore/factory.h"
#include "libmscore/undo.h"
#include "libmscore/excerpt.h"
#include "libmscore/page.h"

#include "igetscore.h"

#include "log.h"
#include "translation.h"

using namespace mu::async;
using namespace mu::notation;
using namespace mu::engraving;

static const mu::engraving::Fraction DEFAULT_TICK = mu::engraving::Fraction(0, 1);

static QString formatInstrumentTitleOnScore(const QString& instrumentName, const Trait& trait, int instrumentNumber)
{
    QString numberPart = instrumentNumber > 0 ? " " + QString::number(instrumentNumber) : QString();

    if (trait.type != TraitType::Transposition || trait.isHiddenOnScore) {
        return instrumentName + numberPart;
    }

    return mu::qtrc("notation", "%1 in %2%3").arg(instrumentName).arg(trait.name).arg(numberPart);
}

static QString formatPartTitle(const Part* part)
{
    QStringList instrumentsNames;
    for (const auto& pair : part->instruments()) {
        instrumentsNames << pair.second->trackName();
    }

    return instrumentsNames.join(" & ");
}

NotationParts::NotationParts(IGetScore* getScore, INotationInteractionPtr interaction, INotationUndoStackPtr undoStack)
    : m_getScore(getScore), m_undoStack(undoStack), m_interaction(interaction)
{
    m_undoStack->undoNotification().onNotify(this, [this]() {
        m_partChangedNotifier.changed();
    });

    m_undoStack->redoNotification().onNotify(this, [this]() {
        m_partChangedNotifier.changed();
    });
}

NotifyList<const Part*> NotationParts::partList() const
{
    NotifyList<const Part*> result;
    result.setNotify(m_partChangedNotifier.notify());

    for (const Part* part: score()->parts()) {
        result.push_back(part);
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

StaffConfig NotationParts::staffConfig(const ID& staffId) const
{
    Staff* staff = staffModifiable(staffId);
    if (!staff) {
        return StaffConfig();
    }

    mu::engraving::StaffType* staffType = staff->staffType(DEFAULT_TICK);
    if (!staffType) {
        return StaffConfig();
    }

    StaffConfig config;
    config.visible = staff->visible();
    config.userDistance = staff->userDist();
    config.cutaway = staff->cutaway();
    config.showIfEmpty = staff->showIfEmpty();
    config.hideSystemBarline = staff->hideSystemBarLine();
    config.mergeMatchingRests = staff->mergeMatchingRests();
    config.hideMode = staff->hideWhenEmpty();
    config.clefTypeList = staff->defaultClefType();

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

    for (Staff* staff : score()->staves()) {
        if (std::find(stavesIds.cbegin(), stavesIds.cend(), staff->id()) != stavesIds.cend()) {
            staves.push_back(staff);
        }
    }

    return staves;
}

std::vector<Part*> NotationParts::parts(const IDList& partsIds) const
{
    std::vector<Part*> parts;

    for (Part* part : score()->parts()) {
        if (std::find(partsIds.cbegin(), partsIds.cend(), part->id()) != partsIds.cend()) {
            parts.push_back(part);
        }
    }

    return parts;
}

mu::engraving::InstrumentChange* NotationParts::findInstrumentChange(const Part* part, const Fraction& tick) const
{
    const mu::engraving::Segment* segment = score()->tick2segment(tick, true, mu::engraving::SegmentType::ChordRest);
    if (!segment) {
        return nullptr;
    }

    mu::engraving::EngravingItem* item = segment->findAnnotation(ElementType::INSTRUMENT_CHANGE, part->startTrack(), part->endTrack());
    return item ? mu::engraving::toInstrumentChange(item) : nullptr;
}

void NotationParts::setParts(const PartInstrumentList& parts, const ScoreOrder& order)
{
    TRACEFUNC;

    endInteractionWithScore();
    startEdit();

    doSetScoreOrder(order);
    removeMissingParts(parts);
    appendNewParts(parts);
    updateSoloist(parts);
    sortParts(parts, score()->staves());
    setBracketsAndBarlines();

    apply();

    m_partChangedNotifier.changed();
}

void NotationParts::setScoreOrder(const ScoreOrder& order)
{
    if (score()->scoreOrder() == order) {
        return;
    }

    startEdit();

    doSetScoreOrder(order);
    setBracketsAndBarlines();

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

    startEdit();

    part->undoChangeProperty(mu::engraving::Pid::VISIBLE, visible);

    apply();

    notifyAboutPartChanged(part);
}

void NotationParts::setPartName(const ID& partId, const QString& name)
{
    TRACEFUNC;

    Part* part = partModifiable(partId);
    if (!part) {
        return;
    }

    if (part->partName() == name) {
        return;
    }

    startEdit();

    score()->undo(new mu::engraving::ChangePart(part, new mu::engraving::Instrument(*part->instrument()), name));

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

    startEdit();

    mu::engraving::Interval oldTransposition = part->instrument()->transpose();

    part->undoChangeProperty(mu::engraving::Pid::PREFER_SHARP_FLAT, shartFlatInt);
    score()->transpositionChanged(part, oldTransposition);

    apply();

    notifyAboutPartChanged(part);
}

void NotationParts::updatePartTitles()
{
    TRACEFUNC;
    for (const Part* part: score()->parts()) {
        setPartName(part->id(), formatPartTitle(part));
    }
}

void NotationParts::doSetScoreOrder(const ScoreOrder& order)
{
    score()->undo(new mu::engraving::ChangeScoreOrder(score(), order));

    m_scoreOrderChanged.notify();
}

void NotationParts::doMoveStaves(const std::vector<Staff*>& staves, staff_idx_t destinationStaffIndex, Part* destinationPart)
{
    TRACEFUNC;

    for (Staff* staff: staves) {
        Staff* movedStaff = staff->clone();

        if (destinationPart) {
            movedStaff->setPart(destinationPart);
        }

        bool needUnlink = !staff->isLinked();

        insertStaff(movedStaff, destinationStaffIndex);
        mu::engraving::Excerpt::cloneStaff(staff, movedStaff);

        if (needUnlink) {
            movedStaff->undoUnlink();
        }

        ++destinationStaffIndex;
    }

    for (Staff* staff: staves) {
        score()->undoRemoveStaff(staff);
    }
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

    std::list<StaffName> newNames { StaffName(name, 0) };
    if (instrument->longNames() == newNames) {
        return;
    }

    startEdit();

    score()->undo(new mu::engraving::ChangeInstrumentLong(instrumentKey.tick, part, newNames));

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

    if (instrument->abbreviature() == abbreviature) {
        return;
    }

    startEdit();

    score()->undo(new mu::engraving::ChangeInstrumentShort(instrumentKey.tick, part, { StaffName(abbreviature, 0) }));

    apply();

    notifyAboutPartChanged(part);
}

bool NotationParts::setVoiceVisible(const ID& staffId, int voiceIndex, bool visible)
{
    TRACEFUNC;

    if (!score()->excerpt()) {
        return false;
    }

    Staff* staff = staffModifiable(staffId);
    if (!staff) {
        return false;
    }

    if (!visible && !staff->canDisableVoice()) {
        return false;
    }

    startEdit();

    score()->excerpt()->setVoiceVisible(staff, voiceIndex, visible);

    apply();

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

    startEdit();

    config.visible = visible;
    doSetStaffConfig(staff, config);

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

    startEdit();

    score()->undo(new mu::engraving::ChangeStaffType(staff, *staffType));

    apply();

    notifyAboutStaffChanged(staff);
}

void NotationParts::setStaffConfig(const ID& staffId, const StaffConfig& config)
{
    TRACEFUNC;

    Staff* staff = staffModifiable(staffId);
    if (!staff) {
        return;
    }

    if (staffConfig(staffId) == config) {
        return;
    }

    startEdit();

    doSetStaffConfig(staff, config);

    apply();

    notifyAboutStaffChanged(staff);
}

void NotationParts::appendStaff(Staff* staff, const ID& destinationPartId)
{
    TRACEFUNC;

    Part* destinationPart = partModifiable(destinationPartId);
    if (!staff || !destinationPart) {
        return;
    }

    startEdit();

    doAppendStaff(staff, destinationPart);
    updateTracks();

    apply();

    notifyAboutStaffAdded(staff, destinationPartId);
}

void NotationParts::appendLinkedStaff(Staff* staff, const ID& sourceStaffId, const mu::ID& destinationPartId)
{
    TRACEFUNC;

    Staff* sourceStaff = staffModifiable(sourceStaffId);
    Part* destinationPart = partModifiable(destinationPartId);
    if (!staff || !sourceStaff || !destinationPart) {
        return;
    }

    startEdit();

    doAppendStaff(staff, destinationPart);

    ///! NOTE: need to unlink before linking
    staff->setLinks(nullptr);
    mu::engraving::Excerpt::cloneStaff(sourceStaff, staff);

    updateTracks();

    apply();

    notifyAboutStaffAdded(staff, destinationPartId);
}

void NotationParts::insertPart(Part* part, size_t index)
{
    TRACEFUNC;

    if (!part) {
        return;
    }

    startEdit();

    doInsertPart(part, static_cast<int>(index));

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

    startEdit();

    size_t partIndex = mu::indexOf(score()->parts(), part);
    score()->cmdRemovePart(part);
    doInsertPart(newPart, partIndex);

    apply();

    notifyAboutPartReplaced(part, newPart);
}

void NotationParts::replaceInstrument(const InstrumentKey& instrumentKey, const Instrument& newInstrument)
{
    TRACEFUNC;

    Part* part = partModifiable(instrumentKey.partId);
    if (!part) {
        return;
    }

    startEdit();

    bool isMainInstrument = part->instrumentId() == instrumentKey.instrumentId;

    if (isMainInstrument) {
        mu::engraving::Interval oldTranspose = part->instrument()->transpose();

        QString newInstrumentPartName = formatInstrumentTitle(newInstrument.trackName(), newInstrument.trait());
        score()->undo(new mu::engraving::ChangePart(part, new mu::engraving::Instrument(newInstrument), newInstrumentPartName));
        score()->transpositionChanged(part, oldTranspose);

        // Update clefs
        for (staff_idx_t staffIdx = 0; staffIdx < part->nstaves(); ++staffIdx) {
            Staff* staff = part->staves().at(staffIdx);
            StaffConfig config = staffConfig(staff->id());

            mu::engraving::ClefTypeList newClefTypeList = newInstrument.clefType(staffIdx);

            if (config.clefTypeList == newClefTypeList) {
                continue;
            }

            config.clefTypeList = newClefTypeList;
            doSetStaffConfig(staff, config);
        }
    } else {
        mu::engraving::InstrumentChange* instrumentChange = findInstrumentChange(part, instrumentKey.tick);
        if (!instrumentChange) {
            rollback();
            return;
        }

        instrumentChange->setInit(true);
        instrumentChange->setupInstrument(&newInstrument);
    }

    apply();

    notifyAboutPartChanged(part);
}

void NotationParts::replaceDrumset(const InstrumentKey& instrumentKey, const Drumset& newDrumset)
{
    Part* part = partModifiable(instrumentKey.partId);
    if (!part) {
        return;
    }

    mu::engraving::Instrument* instrument = part->instrument(instrumentKey.tick);
    if (!instrument) {
        return;
    }

    startEdit();

    score()->undo(new mu::engraving::ChangeDrumset(instrument, &newDrumset));

    apply();

    notifyAboutPartChanged(part);
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

void NotationParts::startEdit()
{
    undoStack()->prepareChanges();
}

void NotationParts::apply()
{
    undoStack()->commitChanges();

    score()->doLayout();
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
    startEdit();

    doRemoveParts(partsToRemove);

    PartInstrumentList parts;
    for (mu::engraving::Part* part: score()->parts()) {
        PartInstrument pi;
        pi.isExistingPart = true;
        pi.partId = part->id();
        parts << pi;
    }

    sortParts(parts, score()->staves());

    setBracketsAndBarlines();

    apply();

    for (const Part* part : partsToRemove) {
        notifyAboutPartRemoved(part);
    }
}

void NotationParts::doRemoveParts(const std::vector<Part*>& parts)
{
    TRACEFUNC;

    for (Part* part : parts) {
        score()->cmdRemovePart(part);
    }
}

void NotationParts::doAppendStaff(Staff* staff, Part* destinationPart)
{
    size_t staffLocalIndex = destinationPart->nstaves();
    mu::engraving::KeyList keyList = score()->keyList();

    staff->setScore(score());
    staff->setPart(destinationPart);

    insertStaff(staff, staffLocalIndex);

    track_idx_t staffGlobalIndex = staff->idx();
    score()->adjustKeySigs(staffGlobalIndex, staffGlobalIndex + 1, keyList);

    setBracketsAndBarlines();

    destinationPart->instrument()->setClefType(staffLocalIndex, staff->defaultClefType());
}

void NotationParts::doSetStaffConfig(Staff* staff, const StaffConfig& config)
{
    mu::engraving::StaffType* staffType = staff->staffType(DEFAULT_TICK);
    if (!staffType) {
        return;
    }

    score()->undo(new mu::engraving::ChangeStaff(staff, config.visible, config.clefTypeList, config.userDistance, config.hideMode,
                                                 config.showIfEmpty, config.cutaway, config.hideSystemBarline, config.mergeMatchingRests));

    score()->undo(new mu::engraving::ChangeStaffType(staff, config.staffType));
}

void NotationParts::doInsertPart(Part* part, size_t index)
{
    TRACEFUNC;

    std::vector<Staff*> stavesCopy(part->staves());
    part->clearStaves();

    mu::engraving::InstrumentList instrumentsCopy = part->instruments();
    part->setInstruments({});

    score()->insertPart(part, index);

    if (score()->excerpt()) {
        score()->excerpt()->parts().insert(score()->excerpt()->parts().begin() + index, part);
    }

    for (auto it = instrumentsCopy.cbegin(); it != instrumentsCopy.cend(); ++it) {
        part->setInstrument(new Instrument(*it->second), it->first);
    }

    for (size_t staffIndex = 0; staffIndex < stavesCopy.size(); ++staffIndex) {
        Staff* staff = stavesCopy[staffIndex];

        Staff* staffCopy = engraving::Factory::createStaff(part);
        staffCopy->setId(staff->id());
        staffCopy->setScore(score());
        staffCopy->setPart(part);
        staffCopy->init(staff);

        insertStaff(staffCopy, static_cast<int>(staffIndex));
        score()->undo(new mu::engraving::Link(staffCopy, staff));

        mu::engraving::Fraction startTick = staff->score()->firstMeasure()->tick();
        mu::engraving::Fraction endTick = staff->score()->lastMeasure()->tick();
        mu::engraving::Excerpt::cloneStaff2(staff, staffCopy, startTick, endTick);
    }

    part->setScore(score());
    updateTracks();
}

void NotationParts::removeStaves(const IDList& stavesIds)
{
    TRACEFUNC;

    std::vector<Staff*> stavesToRemove = staves(stavesIds);
    if (stavesToRemove.empty()) {
        return;
    }

    endInteractionWithScore();
    startEdit();

    for (Staff* staff: stavesToRemove) {
        score()->cmdRemoveStaff(staff->idx());
    }

    setBracketsAndBarlines();

    apply();

    for (const Staff* staff : stavesToRemove) {
        notifyAboutStaffRemoved(staff);
    }
}

void NotationParts::moveParts(const IDList& sourcePartsIds, const ID& destinationPartId, InsertMode mode)
{
    std::vector<Part*> sourceParts = parts(sourcePartsIds);
    if (sourceParts.empty()) {
        return;
    }

    QList<ID> allScorePartIds;
    for (mu::engraving::Part* currentPart: score()->parts()) {
        allScorePartIds.push_back(currentPart->id());
    }

    if (!allScorePartIds.contains(destinationPartId)) {
        return;
    }

    TRACEFUNC;

    for (const Part* sourcePart: sourceParts) {
        int srcIndex = allScorePartIds.indexOf(sourcePart->id());
        int dstIndex = allScorePartIds.indexOf(destinationPartId);
        dstIndex += (mode == InsertMode::Before) && (srcIndex < dstIndex) ? -1 : 0;

        if (dstIndex >= 0 && dstIndex < allScorePartIds.size()) {
            allScorePartIds.move(srcIndex, dstIndex);
        }
    }

    PartInstrumentList parts;
    for (const ID& partId: allScorePartIds) {
        PartInstrument pi;
        pi.isExistingPart = true;
        pi.partId = partId;
        parts << pi;
    }

    endInteractionWithScore();
    startEdit();

    sortParts(parts, score()->staves());

    setBracketsAndBarlines();

    apply();
}

void NotationParts::moveStaves(const IDList& sourceStavesIds, const ID& destinationStaffId, InsertMode mode)
{
    TRACEFUNC;

    if (sourceStavesIds.empty()) {
        return;
    }

    Staff* destinationStaff = staffModifiable(destinationStaffId);
    if (!destinationStaff) {
        return;
    }

    std::vector<Staff*> staves = this->staves(sourceStavesIds);
    if (staves.empty()) {
        return;
    }

    Part* destinationPart = destinationStaff->part();
    staff_idx_t destinationStaffIndex = (mode == InsertMode::Before ? destinationStaff->idx() : destinationStaff->idx() + 1);
    destinationStaffIndex -= score()->staffIdx(destinationPart); // NOTE: convert to local part's staff index

    endInteractionWithScore();
    startEdit();

    doMoveStaves(staves, destinationStaffIndex, destinationPart);

    setBracketsAndBarlines();

    apply();
}

void NotationParts::appendStaves(Part* part, const InstrumentTemplate& templ, const mu::engraving::KeyList& keyList)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(part) {
        return;
    }

    for (staff_idx_t staffIndex = 0; staffIndex < templ.staffCount; ++staffIndex) {
        staff_idx_t lastStaffIndex = !score()->staves().empty() ? score()->staves().back()->idx() : 0;

        Staff* staff = engraving::Factory::createStaff(part);
        const mu::engraving::StaffType* staffType = templ.staffTypePreset;
        if (!staffType) {
            staffType = mu::engraving::StaffType::preset(StaffTypeId::STANDARD);
        }
        initStaff(staff, templ, staffType, staffIndex);

        if (lastStaffIndex > 0) {
            staff->setBarLineSpan(score()->staff(lastStaffIndex - 1)->barLineSpan());
        }

        insertStaff(staff, staffIndex);
    }

    if (!part->nstaves()) {
        return;
    }

    staff_idx_t firstStaffIndex = part->staff(0)->idx();
    staff_idx_t endStaffIndex = firstStaffIndex + part->nstaves();
    score()->adjustKeySigs(firstStaffIndex, endStaffIndex, keyList);
}

void NotationParts::insertStaff(Staff* staff, staff_idx_t destinationStaffIndex)
{
    TRACEFUNC;

    score()->undoInsertStaff(staff, destinationStaffIndex);
}

void NotationParts::initStaff(Staff* staff, const InstrumentTemplate& templ, const mu::engraving::StaffType* staffType, size_t cleffIndex)
{
    TRACEFUNC;

    const mu::engraving::StaffType* staffTypePreset = staffType ? staffType : templ.staffTypePreset;
    if (!staffTypePreset) {
        staffTypePreset = mu::engraving::StaffType::getDefaultPreset(templ.staffGroup);
    }

    mu::engraving::StaffType* stt = staff->setStaffType(DEFAULT_TICK, *staffTypePreset);
    if (cleffIndex >= MAX_STAVES) {
        stt->setSmall(false);
    } else {
        stt->setSmall(templ.smallStaff[cleffIndex]);
        staff->setBracketType(0, templ.bracket[cleffIndex]);
        staff->setBracketSpan(0, templ.bracketSpan[cleffIndex]);
        staff->setBarLineSpan(templ.barlineSpan[cleffIndex]);
    }
    staff->setDefaultClefType(templ.clefType(cleffIndex));
}

void NotationParts::removeMissingParts(const PartInstrumentList& newParts)
{
    TRACEFUNC;

    auto needRemove = [&newParts](const Part* part) {
        for (const PartInstrument& pi : newParts) {
            if (pi.partId == part->id()) {
                return false;
            }
        }

        return true;
    };

    std::vector<Part*> partsToRemove;

    for (Part* part: score()->parts()) {
        if (needRemove(part)) {
            partsToRemove.push_back(part);
        }
    }

    doRemoveParts(partsToRemove);
}

void NotationParts::appendNewParts(const PartInstrumentList& parts)
{
    TRACEFUNC;

    size_t staffCount = 0;
    mu::engraving::KeyList keyList = score()->keyList();

    for (const PartInstrument& pi: parts) {
        if (pi.isExistingPart) {
            staffCount += part(pi.partId)->nstaves();
            continue;
        }

        Instrument instrument = Instrument::fromTemplate(&pi.instrumentTemplate);
        const std::list<StaffName>& longNames = instrument.longNames();
        const std::list<StaffName>& shortNames = instrument.shortNames();

        Part* part = new Part(score());
        part->setSoloist(pi.isSoloist);
        part->setInstrument(instrument);

        int instrumentNumber = resolveNewInstrumentNumber(pi.instrumentTemplate, parts);

        QString formattedPartName = formatInstrumentTitle(instrument.trackName(), instrument.trait(), instrumentNumber);
        QString longName = !longNames.empty() ? longNames.front().name().toQString() : QString();
        QString formattedLongName = formatInstrumentTitleOnScore(longName, instrument.trait(), instrumentNumber);
        QString shortName = !shortNames.empty() ? shortNames.front().name().toQString() : QString();
        QString formattedShortName = formatInstrumentTitleOnScore(shortName, instrument.trait(), instrumentNumber);

        part->setPartName(formattedPartName);
        part->setLongName(formattedLongName);
        part->setShortName(formattedShortName);

        score()->undo(new mu::engraving::InsertPart(part, static_cast<int>(staffCount)));
        appendStaves(part, pi.instrumentTemplate, keyList);
        staffCount += part->nstaves();

        m_partChangedNotifier.itemAdded(part);
    }
}

void NotationParts::updateSoloist(const PartInstrumentList& parts)
{
    TRACEFUNC;

    for (const PartInstrument& pi: parts) {
        Part* part = partModifiable(pi.partId);

        if (pi.isExistingPart && (pi.isSoloist != part->soloist())) {
            score()->undo(new mu::engraving::SetSoloist(part, pi.isSoloist));
        }
    }
}

void NotationParts::sortParts(const PartInstrumentList& parts, const std::vector<mu::engraving::Staff*>& originalStaves)
{
    TRACEFUNC;

    std::vector<mu::engraving::staff_idx_t> staffMapping;
    std::vector<mu::engraving::staff_idx_t> trackMapping;
    int runningStaffIndex = 0;

    int partIndex = 0;
    for (const PartInstrument& pi: parts) {
        mu::engraving::Part* currentPart = pi.isExistingPart ? partModifiable(pi.partId) : score()->parts()[partIndex];

        for (mu::engraving::Staff* staff : currentPart->staves()) {
            mu::engraving::staff_idx_t actualStaffIndex = mu::indexOf(score()->staves(), staff);

            trackMapping.push_back(mu::indexOf(originalStaves, staff));
            staffMapping.push_back(actualStaffIndex);
            ++runningStaffIndex;
        }
        ++partIndex;
    }

    score()->undo(new mu::engraving::SortStaves(score(), staffMapping));

    score()->undo(new mu::engraving::MapExcerptTracks(score(), trackMapping));
}

void NotationParts::updateTracks()
{
    if (!score()->excerpt()) {
        return;
    }

    score()->excerpt()->updateTracksMapping();
}

int NotationParts::resolveNewInstrumentNumber(const InstrumentTemplate& instrument,
                                              const PartInstrumentList& allNewInstruments) const
{
    int count = 0;

    for (const Part* part : score()->parts()) {
        const Instrument* partInstrument = part->instrument();

        if (partInstrument->id() == instrument.id
            && partInstrument->trait().name == instrument.trait.name) {
            ++count;
        }
    }

    if (count > 0) {
        return count + 1;
    }

    for (const PartInstrument& partInstrument: allNewInstruments) {
        const InstrumentTemplate& templ = partInstrument.instrumentTemplate;

        if (templ.id == instrument.id && templ.trait.name == instrument.trait.name) {
            ++count;
        }
    }

    return count > 1 ? 1 : 0;
}

void NotationParts::setBracketsAndBarlines()
{
    score()->setBracketsAndBarlines();
}

void NotationParts::endInteractionWithScore()
{
    m_interaction->clearSelection();
    m_interaction->noteInput()->resetInputPosition();
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

void NotationParts::notifyAboutStaffAdded(const Staff* staff, const ID& partId) const
{
    IF_ASSERT_FAILED(staff) {
        return;
    }

    ChangedNotifier<const Staff*>& notifier = m_staffChangedNotifierMap[partId];
    notifier.itemAdded(staff);
}

void NotationParts::notifyAboutStaffRemoved(const Staff* staff) const
{
    IF_ASSERT_FAILED(staff) {
        return;
    }

    ChangedNotifier<const Staff*>& notifier = m_staffChangedNotifierMap[staff->part()->id()];
    notifier.itemRemoved(staff);
}
