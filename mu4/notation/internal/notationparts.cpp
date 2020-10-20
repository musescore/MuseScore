//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FIT-0NESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "notationparts.h"

#include "libmscore/score.h"
#include "libmscore/undo.h"
#include "libmscore/excerpt.h"
#include "libmscore/drumset.h"
#include "libmscore/instrchange.h"

#include "igetscore.h"

#include "log.h"

using namespace mu::async;
using namespace mu::notation;
using namespace mu::instruments;

NotationParts::NotationParts(IGetScore* getScore, Notification selectionChangedNotification)
    : m_getScore(getScore), m_partsNotifier(new ChangedNotifier<const Part*>())
{
    selectionChangedNotification.onNotify(this, [this]() {
        updateCanChangeInstrumentsVisibility();
    });
}

NotationParts::~NotationParts()
{
    delete m_partsNotifier;
}

Ms::Score* NotationParts::score() const
{
    return m_getScore->score();
}

Ms::MasterScore* NotationParts::masterScore() const
{
    return score()->masterScore();
}

void NotationParts::startEdit()
{
    masterScore()->startCmd();
}

void NotationParts::apply()
{
    masterScore()->endCmd();
}

NotifyList<const Part*> NotationParts::partList() const
{
    NotifyList<const Part*> result;

    std::vector<Part*> parts = availableParts();

    QSet<ID> partIds;
    for (const Part* part: parts) {
        if (partIds.contains(part->id())) {
            continue;
        }

        result.push_back(part);
        partIds.insert(part->id());
    }

    result.setNotify(m_partsNotifier->notify());
    return result;
}

NotifyList<Instrument> NotationParts::instrumentList(const ID& partId) const
{
    Part* part = this->part(partId);
    if (!part) {
        return NotifyList<Instrument>();
    }

    NotifyList<Instrument> result;

    for (const Ms::Instrument* instrument: instruments(part).values()) {
        result.push_back(convertedInstrument(instrument, part));
    }

    ChangedNotifier<Instrument>* notifier = partNotifier(partId);
    result.setNotify(notifier->notify());
    return result;
}

NotifyList<const Staff*> NotationParts::staffList(const ID& partId, const ID& instrumentId) const
{
    Part* part = this->part(partId);
    if (!part) {
        return NotifyList<const Staff*>();
    }

    NotifyList<const Staff*> result;
    std::vector<const Staff*> staves = this->staves(part, instrumentId);
    for (const Staff* staff: staves) {
        result.push_back(staff);
    }

    ChangedNotifier<const Staff*>* notifier = instrumentNotifier(instrumentId, partId);
    result.setNotify(notifier->notify());
    return result;
}

void NotationParts::setInstruments(const InstrumentList& instruments)
{
    IDList instrumentIds;
    for (const Instrument& instrument: instruments) {
        instrumentIds << instrument.id;
    }

    startEdit();
    removeMissingInstruments(instrumentIds);

    IDList existedInstrumentIds = allInstrumentsIds();

    for (const Instrument& instrument: instruments) {
        bool isInstrumentExist = !existedInstrumentIds.isEmpty() && existedInstrumentIds.contains(instrument.id);
        if (isInstrumentExist) {
            continue;
        }

        Part* part = new Part(score());

        part->setPartName(instrument.name);
        part->setInstrument(convertedInstrument(instrument));

        score()->undo(new Ms::InsertPart(part, lastStaffIndex()));
        appendStaves(part, instrument);
    }

    if (score()->measures()->empty()) {
        score()->insertMeasure(ElementType::MEASURE, 0, false);
    }

    sortParts(instrumentIds);

    removeEmptyExcerpts();

    apply();

    m_partsNotifier->changed();
    m_partsChanged.notify();
}

void NotationParts::setPartVisible(const ID& partId, bool visible)
{
    Part* part = this->part(partId);
    if (!part) {
        part = this->part(partId, masterScore());
        if (!part) {
            return;
        }

        appendPart(part);
        m_partsChanged.notify();
        return;
    }

    startEdit();
    part->undoChangeProperty(Ms::Pid::VISIBLE, visible);
    apply();

    m_partsNotifier->itemChanged(part);
    m_partsChanged.notify();
}

void NotationParts::setPartName(const ID& partId, const QString& name)
{
    Part* part = this->part(partId);
    if (!part) {
        return;
    }

    startEdit();
    doSetPartName(part, name);
    apply();

    m_partsNotifier->itemChanged(part);
    m_partsChanged.notify();
}

void NotationParts::setInstrumentVisible(const ID& instrumentId, const ID& fromPartId, bool visible)
{
    Part* part = this->part(fromPartId);
    if (!part) {
        return;
    }

    InstrumentInfo instrumentInfo = this->instrumentInfo(instrumentId, part);
    if (!instrumentInfo.isValid()) {
        return;
    }

    if (needAssignInstrumentToChord(instrumentId, fromPartId)) {
        assignIstrumentToSelectedChord(instrumentInfo.instrument);
        return;
    }

    startEdit();

    std::vector<const Staff*> instrumentStaves = staves(part, instrumentId);
    for (const Staff* staff: instrumentStaves) {
        Staff* _staff = score()->staff(staff->idx());
        doSetStaffVisible(_staff, visible);
    }

    apply();

    ChangedNotifier<Instrument>* notifier = partNotifier(fromPartId);
    notifier->itemChanged(convertedInstrument(instrumentInfo.instrument, part));
    m_partsChanged.notify();
}

Ms::ChordRest* NotationParts::selectedChord() const
{
    Ms::ChordRest* chord = score()->getSelectedChordRest();

    if (Ms::MScore::_error == Ms::MsError::NO_NOTE_REST_SELECTED) {
        Ms::MScore::_error = Ms::MsError::MS_NO_ERROR;
    }

    return chord;
}

void NotationParts::updateCanChangeInstrumentsVisibility()
{
    for (const InstrumentKey& key: m_canChangeInstrumentsVisibilityHash.keys()) {
        bool canChangeVisibility = resolveCanChangeInstrumentVisibility(key.partId, key.instrumentId);
        m_canChangeInstrumentsVisibilityHash[key].ch.send(canChangeVisibility);
    }
}

bool NotationParts::resolveCanChangeInstrumentVisibility(const ID& instrumentId, const ID& fromPartId) const
{
    if (!needAssignInstrumentToChord(instrumentId, fromPartId)) {
        return true;
    }

    const Ms::ChordRest* chord = selectedChord();
    return chord && chord->part()->id() == fromPartId;
}

bool NotationParts::needAssignInstrumentToChord(const ID& instrumentId, const ID& fromPartId) const
{
    Part* part = this->part(fromPartId);
    if (!part) {
        return false;
    }

    bool isMainInstrument = part->instrumentId() == instrumentId;
    if (isMainInstrument) {
        return false;
    }

    QMap<Ms::Fraction, Ms::InstrumentChange*> instrumentChangeElements = this->instrumentChangeElements(fromPartId);

    for (const Ms::InstrumentChange* instrumentChange: instrumentChangeElements.values()) {
        if (instrumentChange->instrument()->instrumentId() == instrumentId) {
            return false;
        }
    }

    return true;
}

void NotationParts::assignIstrumentToSelectedChord(Ms::Instrument* instrument)
{
    Ms::ChordRest* chord = selectedChord();
    if (!chord) {
        return;
    }

    startEdit();
    Part* part = chord->part();
    part->removeInstrument(instrument->instrumentId());
    part->setInstrument(instrument, chord->segment()->tick());

    auto instrumentChange = new Ms::InstrumentChange(*instrument, score());
    instrumentChange->setInit(true);
    instrumentChange->setParent(chord->segment());
    instrumentChange->setTrack((chord->track() / VOICES) * VOICES);
    instrumentChange->setupInstrument(instrument);

    score()->undoAddElement(instrumentChange);
    apply();

    ChangedNotifier<Instrument>* notifier = partNotifier(part->id());
    notifier->itemChanged(convertedInstrument(instrument, part));
    m_partsChanged.notify();
}

void NotationParts::doMovePart(const ID& sourcePartId, const ID& destinationPartId, INotationParts::InsertMode mode)
{
    Part* part = this->part(sourcePartId);
    Part* toPart = this->part(destinationPartId);
    if (!part || !toPart) {
        return;
    }

    bool partIsBefore = score()->staffIdx(part) < score()->staffIdx(toPart);
    QList<Staff*> staves = *part->staves();
    int newStaffIndex = partIsBefore ? staves.size() : 0;

    score()->undoRemovePart(part);

    int toPartIndex = score()->parts().indexOf(toPart);
    int newPartIndex = mode == Before ? toPartIndex : toPartIndex + 1;
    score()->parts().insert(newPartIndex, part);

    for (Staff* staff: staves) {
        Staff* movedStaff = staff->clone();
        score()->undoInsertStaff(movedStaff, newStaffIndex);
        Ms::Excerpt::cloneStaff(staff, movedStaff);
        newStaffIndex++;
    }

    for (Staff* staff: staves) {
        score()->undoRemoveStaff(staff);
    }
}

void NotationParts::setInstrumentName(const ID& instrumentId, const ID& fromPartId, const QString& name)
{
    Part* part = this->part(fromPartId);
    if (!part) {
        return;
    }

    InstrumentInfo instrumentInfo = this->instrumentInfo(instrumentId, part);
    if (!instrumentInfo.isValid()) {
        return;
    }

    startEdit();
    score()->undo(new Ms::ChangeInstrumentLong(instrumentInfo.fraction, part, { StaffName(name, 0) }));
    apply();

    InstrumentInfo newInstrumentInfo = this->instrumentInfo(instrumentId, part);
    ChangedNotifier<Instrument>* notifier = partNotifier(part->id());
    notifier->itemChanged(convertedInstrument(newInstrumentInfo.instrument, part));
    m_partsChanged.notify();
}

void NotationParts::setInstrumentAbbreviature(const ID& instrumentId, const ID& fromPartId, const QString& abbreviature)
{
    Part* part = this->part(fromPartId);
    if (!part) {
        return;
    }

    InstrumentInfo instrumentInfo = this->instrumentInfo(instrumentId, part);
    if (!instrumentInfo.isValid()) {
        return;
    }

    startEdit();
    score()->undo(new Ms::ChangeInstrumentShort(instrumentInfo.fraction, part, { StaffName(abbreviature, 0) }));
    apply();

    InstrumentInfo newInstrumentInfo = this->instrumentInfo(instrumentId, part);
    ChangedNotifier<Instrument>* notifier = partNotifier(part->id());
    notifier->itemChanged(convertedInstrument(newInstrumentInfo.instrument, part));
    m_partsChanged.notify();
}

void NotationParts::setStaffVisible(const ID& staffId, bool visible)
{
    Staff* staff = this->staff(staffId);
    if (!staff) {
        return;
    }

    startEdit();
    doSetStaffVisible(staff, visible);
    apply();

    notifyAboutStaffChanged(staffId);
    m_partsChanged.notify();
}

void NotationParts::doSetStaffVisible(Staff* staff, bool visible)
{
    if (!staff) {
        return;
    }

    staff->setInvisible(!visible);
    score()->undo(new Ms::ChangeStaff(staff));
}

void NotationParts::setStaffType(const ID& staffId, StaffType type)
{
    Staff* staff = this->staff(staffId);
    const Ms::StaffType* staffType = Ms::StaffType::preset(type);

    if (!staff || !staffType) {
        return;
    }

    startEdit();
    score()->undo(new Ms::ChangeStaffType(staff, *staffType));
    apply();

    notifyAboutStaffChanged(staffId);
    m_partsChanged.notify();
}

void NotationParts::setCutawayEnabled(const ID& staffId, bool enabled)
{
    Staff* staff = this->staff(staffId);
    if (!staff) {
        return;
    }

    startEdit();
    staff->setCutaway(enabled);
    score()->undo(new Ms::ChangeStaff(staff));
    apply();

    notifyAboutStaffChanged(staffId);
    m_partsChanged.notify();
}

void NotationParts::setSmallStaff(const ID& staffId, bool smallStaff)
{
    Staff* staff = this->staff(staffId);
    Ms::StaffType* staffType = staff->staffType(Ms::Fraction(0, 1));

    if (!staff || !staffType) {
        return;
    }

    startEdit();
    staffType->setSmall(smallStaff);
    score()->undo(new Ms::ChangeStaffType(staff, *staffType));
    apply();

    notifyAboutStaffChanged(staffId);
    m_partsChanged.notify();
}

void NotationParts::setVoiceVisible(const ID& staffId, int voiceIndex, bool visible)
{
    Staff* staff = this->staff(staffId);
    if (!staff) {
        return;
    }

    startEdit();

    Ms::SegmentType segmentType = Ms::SegmentType::ChordRest;
    for (const Ms::Segment* segment = score()->firstSegment(segmentType); segment; segment = segment->next1(segmentType)) {
        for (Ms::Element* element: segment->elist()) {
            if (!element) {
                continue;
            }

            if (element->staffIdx() == staff->idx() && element->voice() == voiceIndex) {
                element->undoChangeProperty(Ms::Pid::VISIBLE, visible);
            }
        }
    }

    staff->setVoiceVisible(voiceIndex, visible);

    apply();

    notifyAboutStaffChanged(staffId);
    m_partsChanged.notify();
}

void NotationParts::appendDoublingInstrument(const Instrument& instrument, const ID& destinationPartId)
{
    Part* part = this->part(destinationPartId);
    if (!part) {
        return;
    }

    int lastTick = 1;
    for (const Ms::Fraction& fraction: instruments(part).keys()) {
        lastTick = std::max(fraction.ticks(), lastTick);
    }

    startEdit();
    part->setInstrument(convertedInstrument(instrument), Ms::Fraction::fromTicks(lastTick + 1));
    doSetPartName(part, formatPartName(part));
    apply();

    ChangedNotifier<Instrument>* notifier = partNotifier(destinationPartId);
    notifier->itemAdded(instrument);
    m_partsNotifier->itemChanged(part);
    m_partsChanged.notify();
}

void NotationParts::appendStaff(const ID& destinationPartId)
{
    Part* part = this->part(destinationPartId);
    if (!part) {
        return;
    }

    InstrumentInfo instrumentInfo = this->instrumentInfo(part->instrumentId(), part);
    if (!instrumentInfo.isValid()) {
        return;
    }

    Ms::Instrument* instrument = instrumentInfo.instrument;
    const QList<Staff*>* instrumentStaves = part->staves();

    startEdit();
    Staff* staff = instrumentStaves->front()->clone();
    staff->setId(Staff::makeId());
    int staffIndex = staff->part()->nstaves();
    score()->undoInsertStaff(staff, staffIndex);
    instrument->setClefType(staffIndex, staff->defaultClefType());
    apply();

    ChangedNotifier<const Staff*>* notifier = instrumentNotifier(instrument->instrumentId(), destinationPartId);
    notifier->itemAdded(staff);
    m_partsChanged.notify();
}

void NotationParts::appendLinkedStaff(const ID& originStaffId)
{
    Staff* staff = this->staff(originStaffId);
    if (!staff || !staff->part()) {
        return;
    }

    Staff* linkedStaff = staff->clone();
    linkedStaff->setId(Staff::makeId());
    int linkedStaffIndex = staff->part()->nstaves();
    staff->linkTo(linkedStaff);

    startEdit();
    score()->undoInsertStaff(linkedStaff, linkedStaffIndex);
    apply();

    InstrumentInfo instrumentInfo = this->instrumentInfo(linkedStaff);
    ChangedNotifier<const Staff*>* notifier = instrumentNotifier(instrumentInfo.instrument->instrumentId(), linkedStaff->part()->id());
    notifier->itemAdded(linkedStaff);
    m_partsChanged.notify();
}

void NotationParts::replaceInstrument(const ID& instrumentId, const ID& fromPartId, const Instrument& newInstrument)
{
    Part* part = this->part(fromPartId);
    if (!part) {
        return;
    }

    InstrumentInfo oldInstrumentInfo = this->instrumentInfo(instrumentId, part);
    if (!oldInstrumentInfo.isValid()) {
        return;
    }

    startEdit();
    part->setInstrument(convertedInstrument(newInstrument), oldInstrumentInfo.fraction);
    doSetPartName(part, formatPartName(part));
    apply();

    ChangedNotifier<Instrument>* notifier = partNotifier(part->id());
    notifier->itemReplaced(convertedInstrument(oldInstrumentInfo.instrument, part), newInstrument);

    m_partsNotifier->itemChanged(part);
    m_partsChanged.notify();
}

Notification NotationParts::partsChanged() const
{
    return m_partsChanged;
}

void NotationParts::removeParts(const IDList& partsIds)
{
    if (partsIds.empty()) {
        return;
    }

    startEdit();
    doRemoveParts(partsIds);
    apply();

    m_partsChanged.notify();
}

void NotationParts::doRemoveParts(const IDList& partsIds)
{
    for (const ID& partId: partsIds) {
        score()->cmdRemovePart(part(partId));
    }
}

void NotationParts::removeInstruments(const IDList& instrumentIds, const ID& fromPartId)
{
    Part* part = this->part(fromPartId);
    if (!part) {
        return;
    }

    startEdit();
    doRemoveInstruments(instrumentIds, part);
    doSetPartName(part, formatPartName(part));
    apply();

    m_partsNotifier->itemChanged(part);
    m_partsChanged.notify();
}

void NotationParts::doRemoveInstruments(const IDList& instrumentIds, Part* fromPart)
{
    QMap<Ms::Fraction, Ms::InstrumentChange*> instrumentChangeElements = this->instrumentChangeElements(fromPart->id());

    for (const ID& instrumentId: instrumentIds) {
        InstrumentInfo instrumentInfo = this->instrumentInfo(instrumentId, fromPart);
        if (!instrumentInfo.isValid()) {
            continue;
        }

        auto instrumentChange = instrumentChangeElements[instrumentInfo.fraction];
        if (instrumentChange) {
            score()->undoRemoveElement(instrumentChange);
        }

        fromPart->removeInstrument(instrumentId);
    }
}

void NotationParts::removeStaves(const IDList& stavesIds)
{
    if (stavesIds.empty()) {
        return;
    }

    startEdit();

    for (Staff* staff: staves(stavesIds)) {
        score()->cmdRemoveStaff(staff->idx());
    }

    apply();

    m_partsChanged.notify();
}

void NotationParts::doSetPartName(Part* part, const QString& name)
{
    score()->undo(new Ms::ChangePart(part, new Ms::Instrument(*part->instrument()), name));
}

void NotationParts::moveParts(const IDList& sourcePartsIds, const ID& destinationPartId, InsertMode mode)
{
    startEdit();

    for (const ID& sourcePartId: sourcePartsIds) {
        doMovePart(sourcePartId, destinationPartId, mode);
    }

    apply();

    m_partsChanged.notify();
}

void NotationParts::moveInstruments(const IDList& sourceInstrumentsIds, const ID& sourcePartId, const ID& destinationPartId,
                                    const ID& destinationInstrumentId, InsertMode mode)
{
    Part* fromPart = part(sourcePartId);
    Part* toPart = part(destinationPartId);

    if (!fromPart || !toPart) {
        return;
    }

    QMap<Ms::Fraction, Ms::Instrument*> movingInstruments = instruments(fromPart, sourceInstrumentsIds);

    startEdit();
    doRemoveInstruments(sourceInstrumentsIds, fromPart);
    doInsertInstruments(movingInstruments, destinationPartId, destinationInstrumentId, mode);

    doSetPartName(fromPart, formatPartName(fromPart));
    if (fromPart != toPart) {
        doSetPartName(toPart, formatPartName(toPart));
    }
    apply();

    m_partsNotifier->itemChanged(fromPart);
    notifyAboutInstrumentsChanged(fromPart->id());
    if (fromPart != toPart) {
        notifyAboutInstrumentsChanged(toPart->id());
        m_partsNotifier->itemChanged(toPart);
    }

    m_partsChanged.notify();
}

QMap<Ms::Fraction, Ms::InstrumentChange*> NotationParts::instrumentChangeElements(const QString& partId) const
{
    QMap<Ms::Fraction, Ms::InstrumentChange*> result;

    Ms::SegmentType segmentType = Ms::SegmentType::ChordRest;
    for (const Ms::Segment* segment = score()->firstSegment(segmentType); segment; segment = segment->next1(segmentType)) {
        for (Ms::Element* element: segment->annotations()) {
            if (!element) {
                continue;
            }

            if (element->part()->id() != partId) {
                continue;
            }

            auto instrumentChange = dynamic_cast<Ms::InstrumentChange*>(element);
            if (!instrumentChange) {
                continue;
            }

            result.insert(instrumentChange->tick(), instrumentChange);
        }
    }

    return result;
}

Ms::ChordRest* NotationParts::chordRest(const Ms::Fraction& fraction, const Part* fromPart) const
{
    Ms::ChordRest* chord = nullptr;
    Ms::SegmentType segmentType = Ms::SegmentType::ChordRest;
    for (const Ms::Segment* segment = score()->firstSegment(segmentType); segment; segment = segment->next1(segmentType)) {
        for (Ms::Element* element: segment->elist()) {
            if (!element) {
                continue;
            }

            if (element->part()->id() != fromPart->id()) {
                continue;
            }

            auto elementChord = dynamic_cast<Ms::ChordRest*>(element);
            if (elementChord && elementChord->tick() == fraction) {
                chord = elementChord;
                break;
            }
        }
        if (chord) {
            break;
        }
    }

    return chord;
}

QMap<Ms::Fraction, Ms::Instrument*> NotationParts::instruments(const Part* fromPart, const IDList& filterInstrumentsIds) const
{
    QMap<Ms::Fraction, Ms::Instrument*> result;

    auto partInstruments = fromPart->instruments();
    for (auto it = partInstruments->begin(); it != partInstruments->end(); it++) {
        Ms::Fraction fraction = Ms::Fraction::fromTicks(it->first);
        Ms::Instrument* instrument = it->second;

        bool acceptedByFilter = !filterInstrumentsIds.isEmpty() ? filterInstrumentsIds.contains(instrument->instrumentId()) : true;
        if (acceptedByFilter) {
            result.insert(fraction, instrument);
        }
    }

    return result;
}

void NotationParts::doInsertInstruments(const QMap<Ms::Fraction, Ms::Instrument*>& instruments,
                                        const ID& destinationPartId, const ID& destinationInstrumentId,
                                        INotationParts::InsertMode mode)
{
    Part* destinationPart = part(destinationPartId);
    if (!destinationPart) {
        return;
    }

    QMap<Ms::Fraction, Ms::Instrument*> partInstrumentsMap = this->instruments(destinationPart);
    QList<Ms::Fraction> partInstrumentsFractions = partInstrumentsMap.keys();
    QList<Ms::Instrument*> partInstruments = partInstrumentsMap.values();

    int destinationIndex = 0;
    for (int i = 0; i < partInstruments.size(); i++) {
        if (partInstruments[i]->instrumentId() == destinationInstrumentId) {
            destinationIndex = i;
            break;
        }
    }

    int newInstrumentIndex = (mode == Before ? destinationIndex : destinationIndex + 1);

    for (Ms::Instrument* instrument: instruments.values()) {
        partInstruments.insert(newInstrumentIndex++, new Ms::Instrument(*instrument));
    }

    for (const Ms::Fraction& fraction: instruments.keys()) {
        if (partInstrumentsFractions.contains(fraction)) {
            partInstrumentsFractions << Ms::Fraction::fromTicks(partInstrumentsFractions.last().ticks() + 1);
            continue;
        }

        partInstrumentsFractions << fraction;
    }

    std::sort(partInstrumentsFractions.begin(), partInstrumentsFractions.end(), [](const Ms::Fraction& l, const Ms::Fraction& r) {
        return l < r;
    });

    if (partInstrumentsFractions.size() > 0) {
        destinationPart->setInstrument(partInstruments[0]);
    }

    QMap<Ms::Fraction, Ms::InstrumentChange*> instrumentChangeElements = this->instrumentChangeElements(destinationPart->id());
    for (int i = 1; i < partInstrumentsFractions.size(); i++) {
        Ms::Instrument* instrument = partInstruments[i];
        Ms::Fraction fraction = partInstrumentsFractions[i];

        Ms::InstrumentChange* instrumentChange = nullptr;
        if (instrumentChangeElements.contains(fraction)) {
            instrumentChange = instrumentChangeElements[fraction];
            score()->undoRemoveElement(instrumentChange);
        } else {
            Ms::ChordRest* chordRest = this->chordRest(fraction, destinationPart);

            if (chordRest) {
                instrumentChange = new Ms::InstrumentChange(*instrument, score());
                instrumentChange->setInit(true);
                instrumentChange->setParent(chordRest->segment());
                instrumentChange->setTrack((chordRest->track() / VOICES) * VOICES);
            } else {
                LOGE() << "Not found chord rest for instrument";
            }
        }

        destinationPart->setInstrument(instrument, fraction);
        if (instrumentChange) {
            instrumentChange->setupInstrument(instrument);
            score()->undoAddElement(instrumentChange);
        }
    }

    doSetPartName(destinationPart, formatPartName(destinationPart));
}

void NotationParts::moveStaves(const IDList& sourceStavesIds, const ID& destinationStaffId, InsertMode mode)
{
    if (sourceStavesIds.empty()) {
        return;
    }

    Staff* toStaff = staff(destinationStaffId);
    if (!toStaff) {
        return;
    }

    std::vector<Staff*> staves = this->staves(sourceStavesIds);
    Part* toPart = toStaff->part();
    int newStaffIndex = (mode == Before ? toStaff->idx() : toStaff->idx() + 1);
    newStaffIndex -= score()->staffIdx(toPart); // NOTE: convert to local part's staff index

    startEdit();
    for (Staff* staff: staves) {
        Staff* movedStaff = staff->clone();
        movedStaff->setPart(toPart);
        score()->undoInsertStaff(movedStaff, newStaffIndex);
        Ms::Excerpt::cloneStaff(staff, movedStaff);
        newStaffIndex++;
    }

    for (Staff* staff: staves) {
        score()->undoRemoveStaff(staff);
    }
    apply();

    m_partsChanged.notify();
}

mu::ValCh<bool> NotationParts::canChangeInstrumentVisibility(const ID& instrumentId, const ID& fromPartId) const
{
    InstrumentKey key { fromPartId, instrumentId };

    if (!m_canChangeInstrumentsVisibilityHash.contains(key)) {
        m_canChangeInstrumentsVisibilityHash[key].val = resolveCanChangeInstrumentVisibility(fromPartId, instrumentId);
    }

    return m_canChangeInstrumentsVisibilityHash[key];
}

std::vector<Part*> NotationParts::availableParts() const
{
    std::vector<Part*> parts;

    std::vector<Part*> scoreParts = this->scoreParts(score());
    parts.insert(parts.end(), scoreParts.begin(), scoreParts.end());

    std::vector<Part*> excerptParts = this->excerptParts(score());
    parts.insert(parts.end(), excerptParts.begin(), excerptParts.end());

    return parts;
}

std::vector<Part*> NotationParts::scoreParts(const Ms::Score* score) const
{
    std::vector<Part*> result;

    for (Part* part: score->parts()) {
        result.push_back(part);
    }

    return result;
}

std::vector<Part*> NotationParts::excerptParts(const Ms::Score* score) const
{
    if (!score->isMaster()) {
        return std::vector<Part*>();
    }

    std::vector<Part*> result;

    for (const Ms::Excerpt* excerpt: score->excerpts()) {
        for (Part* part: excerpt->parts()) {
            result.push_back(part);
        }
    }

    return result;
}

Part* NotationParts::part(const ID& partId, const Ms::Score* score) const
{
    if (!score) {
        score = this->score();
    }

    std::vector<Part*> parts = availableParts();

    for (Part* part: parts) {
        if (part->id() == partId) {
            return part;
        }
    }

    LOGW() << QString("Part %1 not found").arg(partId);
    return nullptr;
}

NotationParts::InstrumentInfo NotationParts::instrumentInfo(const ID& instrumentId, const Part* fromPart) const
{
    if (!fromPart) {
        return InstrumentInfo();
    }

    auto partInstruments = instruments(fromPart);
    if (partInstruments.isEmpty()) {
        return InstrumentInfo();
    }

    for (const Ms::Fraction& fraction: partInstruments.keys()) {
        Ms::Instrument* instrument = partInstruments.value(fraction);
        if (instrument->instrumentId() == instrumentId) {
            return InstrumentInfo(fraction, instrument);
        }
    }

    return InstrumentInfo();
}

NotationParts::InstrumentInfo NotationParts::instrumentInfo(const Staff* staff) const
{
    if (!staff || !staff->part()) {
        return InstrumentInfo();
    }

    return InstrumentInfo(Ms::Fraction(-1, 1), staff->part()->instrument());
}

Staff* NotationParts::staff(const ID& staffId) const
{
    Staff* staff = score()->staff(staffId);

    if (!staff) {
        LOGW() << QString("Staff %1 not found").arg(staffId);
    }

    return staff;
}

std::vector<const Staff*> NotationParts::staves(const Part* part, const ID& instrumentId) const
{
    // TODO: configure staves by instrumentId
    Q_UNUSED(instrumentId)

    std::vector<const Staff*> result;

    for (const Staff* staff: *part->staves()) {
        result.push_back(staff);
    }

    return result;
}

std::vector<Staff*> NotationParts::staves(const IDList& stavesIds) const
{
    std::vector<Staff*> staves;

    for (const ID& staffId: stavesIds) {
        Staff* staff = this->staff(staffId);

        if (staff) {
            staves.push_back(staff);
        }
    }

    return staves;
}

void NotationParts::appendPart(Part* part)
{
    for (Staff* partStaff: *part->staves()) {
        Staff* staff = new Staff(score());
        staff->setPart(part);
        staff->init(partStaff);
        if (partStaff->links() && !part->staves()->isEmpty()) {
            Staff* linkedStaff = part->staves()->back();
            staff->linkTo(linkedStaff);
        }
        part->insertStaff(staff, -1);
        score()->staves().append(staff);
    }

    score()->appendPart(part);
}

void NotationParts::appendStaves(Part* part, const Instrument& instrument)
{
    for (int staffIndex = 0; staffIndex < instrument.staves; ++staffIndex) {
        int lastStaffIndex = this->lastStaffIndex();

        Staff* staff = new Staff(score());
        staff->setPart(part);
        initStaff(staff, instrument, Ms::StaffType::preset(StaffType::STANDARD), staffIndex);

        if (lastStaffIndex > 0) {
            staff->setBarLineSpan(score()->staff(lastStaffIndex - 1)->barLineSpan());
        }

        score()->undoInsertStaff(staff, staffIndex);
    }
}

void NotationParts::removeMissingInstruments(const IDList& newInstrumentIds)
{
    IDList partsToRemove;

    for (const Part* part: partList()) {
        auto partInstruments = instruments(part);
        IDList instrumentsToRemove;

        for (const Ms::Instrument* instrument: partInstruments.values()) {
            if (!newInstrumentIds.contains(instrument->instrumentId())) {
                instrumentsToRemove << instrument->instrumentId();
            }
        }

        bool removeAllInstruments = instrumentsToRemove.size() == partInstruments.size();
        if (removeAllInstruments) {
            partsToRemove << part->id();
        } else {
            doRemoveInstruments(instrumentsToRemove, this->part(part->id()));
        }
    }

    doRemoveParts(partsToRemove);
}

IDList NotationParts::allInstrumentsIds() const
{
    IDList result;

    for (const Part* part: partList()) {
        auto partInstruments = instruments(part);

        for (const Ms::Instrument* instrument: partInstruments.values()) {
            result << instrument->instrumentId();
        }
    }

    return result;
}

int NotationParts::lastStaffIndex() const
{
    return !score()->staves().isEmpty() ? score()->staves().last()->idx() : 0;
}

void NotationParts::removeEmptyExcerpts()
{
    const QList<Ms::Excerpt*> excerpts(masterScore()->excerpts());
    for (Ms::Excerpt* excerpt: excerpts) {
        QList<Staff*> staves = excerpt->partScore()->staves();

        if (staves.empty()) {
            masterScore()->undo(new Ms::RemoveExcerpt(excerpt));
        }
    }
}

Ms::Instrument NotationParts::convertedInstrument(const Instrument& instrument) const
{
    Ms::Instrument museScoreInstrument;
    museScoreInstrument.setAmateurPitchRange(instrument.amateurPitchRange.min, instrument.amateurPitchRange.max);
    museScoreInstrument.setProfessionalPitchRange(instrument.professionalPitchRange.min, instrument.professionalPitchRange.max);
    for (Ms::StaffName sn: instrument.longNames) {
        museScoreInstrument.addLongName(StaffName(sn.name(), sn.pos()));
    }
    for (Ms::StaffName sn: instrument.shortNames) {
        museScoreInstrument.addShortName(StaffName(sn.name(), sn.pos()));
    }
    museScoreInstrument.setTrackName(instrument.name);
    museScoreInstrument.setTranspose(instrument.transpose);
    museScoreInstrument.setInstrumentId(instrument.id);
    if (instrument.useDrumset) {
        museScoreInstrument.setDrumset(instrument.drumset ? instrument.drumset : Ms::smDrumset);
    }
    for (int i = 0; i < instrument.staves; ++i) {
        museScoreInstrument.setClefType(i, instrument.clefs[i]);
    }
    museScoreInstrument.setMidiActions(convertedMidiActions(instrument.midiActions));
    museScoreInstrument.setArticulation(instrument.midiArticulations);
    for (const instruments::Channel& c : instrument.channels) {
        museScoreInstrument.appendChannel(new instruments::Channel(c));
    }
    museScoreInstrument.setStringData(instrument.stringData);
    museScoreInstrument.setSingleNoteDynamics(instrument.singleNoteDynamics);
    return museScoreInstrument;
}

Instrument NotationParts::convertedInstrument(const Ms::Instrument* museScoreInstrument, const Part* part) const
{
    Instrument instrument;
    instrument.amateurPitchRange = PitchRange(museScoreInstrument->minPitchA(), museScoreInstrument->maxPitchA());
    instrument.professionalPitchRange = PitchRange(museScoreInstrument->minPitchP(), museScoreInstrument->maxPitchP());
    for (Ms::StaffName sn: museScoreInstrument->longNames()) {
        instrument.longNames << StaffName(sn.name(), sn.pos());
    }
    for (Ms::StaffName sn: museScoreInstrument->shortNames()) {
        instrument.shortNames << StaffName(sn.name(), sn.pos());
    }
    instrument.name = museScoreInstrument->trackName();
    instrument.transpose = museScoreInstrument->transpose();
    instrument.id = museScoreInstrument->instrumentId();
    instrument.useDrumset = museScoreInstrument->useDrumset();
    instrument.drumset = museScoreInstrument->drumset();
    for (int i = 0; i < museScoreInstrument->cleffTypeCount(); ++i) {
        instrument.clefs[i] = museScoreInstrument->clefType(i);
    }
    instrument.midiActions = convertedMidiActions(museScoreInstrument->midiActions());
    instrument.midiArticulations = museScoreInstrument->articulation();
    for (instruments::Channel* c : museScoreInstrument->channel()) {
        instrument.channels.append(*c);
    }
    instrument.stringData = *museScoreInstrument->stringData();
    instrument.singleNoteDynamics = museScoreInstrument->singleNoteDynamics();
    instrument.visible = isInstrumentVisible(instrument.id, part);
    instrument.isDoubling = part->instrument()->instrumentId() != instrument.id;
    return instrument;
}

bool NotationParts::isInstrumentVisible(const ID& instrumentId, const Part* fromPart) const
{
    for (const Staff* staff: staves(fromPart, instrumentId)) {
        if (!staff->invisible()) {
            return true;
        }
    }

    return false;
}

void NotationParts::initStaff(Staff* staff, const Instrument& instrument, const Ms::StaffType* staffType, int cleffIndex)
{
    const Ms::StaffType* pst = staffType ? staffType : instrument.staffTypePreset;
    if (!pst) {
        pst = Ms::StaffType::getDefaultPreset(instrument.staffGroup);
    }

    Ms::StaffType* stt = staff->setStaffType(Ms::Fraction(0, 1), *pst);
    if (cleffIndex >= MAX_STAVES) {
        stt->setSmall(false);
    } else {
        stt->setSmall(instrument.smallStaff[cleffIndex]);
        staff->setBracketType(0, instrument.bracket[cleffIndex]);
        staff->setBracketSpan(0, instrument.bracketSpan[cleffIndex]);
        staff->setBarLineSpan(instrument.barlineSpan[cleffIndex]);
    }
    staff->setDefaultClefType(instrument.clefs[cleffIndex]);
}

QList<Ms::NamedEventList> NotationParts::convertedMidiActions(const MidiActionList& midiActions) const
{
    QList<Ms::NamedEventList> result;

    for (const MidiAction& action: midiActions) {
        Ms::NamedEventList event;
        event.name = action.name;
        event.descr = action.description;

        for (const midi::Event& midiEvent: action.events) {
            Ms::MidiCoreEvent midiCoreEvent;
            midiCoreEvent.setType(static_cast<uchar>(midiEvent.type()));
            midiCoreEvent.setChannel(midiCoreEvent.channel());
            //!FIXME
            //midiCoreEvent.setData(midiEvent.a, midiEvent.b);
            event.events.push_back(midiCoreEvent);
        }
    }

    return result;
}

MidiActionList NotationParts::convertedMidiActions(const QList<Ms::NamedEventList>& midiActions) const
{
    MidiActionList result;

    for (const Ms::NamedEventList& coreAction: midiActions) {
        MidiAction action;
        action.name = coreAction.name;
        action.description = coreAction.descr;

        for (const Ms::MidiCoreEvent& midiCoreEvent: coreAction.events) {
            midi::Event midiEvent(midiCoreEvent.channel(),
                                  static_cast<midi::EventType>(midiCoreEvent.type()),
                                  midiCoreEvent.dataA(),
                                  midiCoreEvent.dataB()
                                  );

            action.events.push_back(midiEvent);
        }
    }

    return result;
}

void NotationParts::sortParts(const IDList& instrumentIds)
{
    Q_ASSERT(score()->parts().size() == static_cast<int>(instrumentIds.size()));

    auto mainInstrumentId = [](const Part* part) {
                                return part->instrument()->instrumentId();
                            };

    for (int i = 0; i < instrumentIds.size(); ++i) {
        const Part* currentPart = score()->parts().at(i);

        if (mainInstrumentId(currentPart) == instrumentIds.at(i)) {
            continue;
        }

        for (int j = i; j < score()->parts().size(); ++j) {
            const Part* part = score()->parts().at(j);

            if (mainInstrumentId(part) == instrumentIds.at(i)) {
                doMovePart(part->id(), currentPart->id());
                break;
            }
        }
    }
}

void NotationParts::notifyAboutStaffChanged(const ID& staffId) const
{
    Staff* staff = this->staff(staffId);
    if (!staff) {
        return;
    }

    InstrumentInfo instrumentInfo = this->instrumentInfo(staff);
    ChangedNotifier<const Staff*>* notifier = instrumentNotifier(instrumentInfo.instrument->instrumentId(), staff->part()->id());
    notifier->itemChanged(staff);
}

void NotationParts::notifyAboutInstrumentsChanged(const ID& partId) const
{
    auto instruments = instrumentList(partId);
    ChangedNotifier<Instrument>* notifier = partNotifier(partId);
    for (const Instrument& instrument: instruments) {
        notifier->itemChanged(instrument);
    }
}

ChangedNotifier<Instrument>* NotationParts::partNotifier(const ID& partId) const
{
    if (m_partsNotifiersMap.find(partId) != m_partsNotifiersMap.end()) {
        return m_partsNotifiersMap[partId];
    }

    ChangedNotifier<Instrument>* notifier = new ChangedNotifier<Instrument>();
    auto value = std::pair<ID, ChangedNotifier<Instrument>*>(partId, notifier);
    m_partsNotifiersMap.insert(value);
    return notifier;
}

ChangedNotifier<const Staff*>* NotationParts::instrumentNotifier(const ID& instrumentId, const ID& fromPartId) const
{
    InstrumentKey key { fromPartId, instrumentId };

    if (!m_instrumentsNotifiersHash.contains(key)) {
        m_instrumentsNotifiersHash[key] = new ChangedNotifier<const Staff*>();
    }

    return m_instrumentsNotifiersHash[key];
}

QString NotationParts::formatPartName(const Part* part) const
{
    QStringList instrumentsNames;
    for (const Ms::Instrument* instrument: instruments(part).values()) {
        instrumentsNames << instrument->trackName();
    }

    return instrumentsNames.join(" & ");
}
