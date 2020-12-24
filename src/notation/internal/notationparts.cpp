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
#include "libmscore/page.h"

#include "instrumentsconverter.h"

#include "igetscore.h"

#include "log.h"

using namespace mu::async;
using namespace mu::notation;
using namespace mu::instruments;

static const Ms::Fraction DEFAULT_TICK = Ms::Fraction(0, 1);

NotationParts::NotationParts(IGetScore* getScore, INotationInteractionPtr interaction, INotationUndoStackPtr undoStack)
    : m_getScore(getScore), m_undoStack(undoStack), m_partsNotifier(new ChangedNotifier<const Part*>())
{
    interaction->selectionChanged().onNotify(this, [this]() {
        updateCanChangeInstrumentsVisibility();
    });

    interaction->dropChanged().onNotify(this, [this]() {
        updatePartTitles();
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

void NotationParts::updateScore()
{
    score()->doLayout();
    m_partsChanged.notify();
}

NotifyList<const Part*> NotationParts::partList() const
{
    NotifyList<const Part*> result;

    std::vector<Part*> parts = availableParts(score());

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

NotifyList<mu::instruments::Instrument> NotationParts::instrumentList(const ID& partId) const
{
    Part* part = this->part(partId);
    if (!part) {
        return NotifyList<mu::instruments::Instrument>();
    }

    NotifyList<mu::instruments::Instrument> result;

    for (const Ms::Instrument* instrument: instruments(part).values()) {
        result.push_back(InstrumentsConverter::convertInstrument(*instrument));
    }

    ChangedNotifier<mu::instruments::Instrument>* notifier = partNotifier(partId);
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

void NotationParts::setInstruments(const mu::instruments::InstrumentList& instruments)
{
    IDList instrumentIds;
    for (const mu::instruments::Instrument& instrument : instruments) {
        instrumentIds << instrument.id;
    }

    removeMissingInstruments(instruments);
    appendNewInstruments(instruments);

    if (score()->measures()->empty()) {
        score()->insertMeasure(ElementType::MEASURE, 0, false);
    }

    sortParts(instruments);

    updateScore();

    m_partsNotifier->changed();
}

void NotationParts::setPartVisible(const ID& partId, bool visible)
{
    Part* part = this->part(partId);

    if (part && part->show() == visible) {
        return;
    }

    if (!part) {
        if (!visible) {
            return;
        }

        part = this->part(partId, masterScore());
        if (!part) {
            return;
        }

        appendPart(part);
        return;
    }

    part->undoChangeProperty(Ms::Pid::VISIBLE, visible);
    updateScore();

    m_partsNotifier->itemChanged(part);
}

void NotationParts::setPartName(const ID& partId, const QString& name)
{
    Part* part = this->part(partId);
    if (!part || part->partName() == name) {
        return;
    }

    doSetPartName(part, name);
    updateScore();

    m_partsNotifier->itemChanged(part);
}

void NotationParts::setPartSharpFlat(const ID& partId, const SharpFlat& sharpFlat)
{
    Part* part = this->part(partId);
    if (!part) {
        return;
    }

    part->undoChangeProperty(Ms::Pid::PREFER_SHARP_FLAT, static_cast<int>(sharpFlat));
    updateScore();

    m_partsNotifier->itemChanged(part);
}

void NotationParts::setPartTransposition(const ID& partId, const instruments::Interval& transpose)
{
    Part* part = this->part(partId);
    if (!part) {
        return;
    }

    score()->transpositionChanged(part, transpose);
    updateScore();

    m_partsNotifier->itemChanged(part);
}

void NotationParts::setInstrumentVisible(const ID& instrumentId, const ID& fromPartId, bool visible)
{
    Part* part = this->part(fromPartId);
    if (!part) {
        return;
    }

    if (part->show() == visible) {
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

    std::vector<const Staff*> instrumentStaves = staves(part, instrumentId);
    for (const Staff* staff: instrumentStaves) {
        Staff* _staff = score()->staff(staff->idx());
        doSetStaffVisible(_staff, visible);
    }

    updateScore();

    ChangedNotifier<mu::instruments::Instrument>* notifier = partNotifier(fromPartId);
    notifier->itemChanged(InstrumentsConverter::convertInstrument(*instrumentInfo.instrument));
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

    Part* part = chord->part();
    part->removeInstrument(instrument->instrumentId());
    part->setInstrument(instrument, chord->segment()->tick());

    auto instrumentChange = new Ms::InstrumentChange(*instrument, score());
    instrumentChange->setInit(true);
    instrumentChange->setParent(chord->segment());
    instrumentChange->setTrack((chord->track() / VOICES) * VOICES);
    instrumentChange->setupInstrument(instrument);

    score()->undoAddElement(instrumentChange);
    updateScore();

    ChangedNotifier<mu::instruments::Instrument>* notifier = partNotifier(part->id());
    notifier->itemChanged(InstrumentsConverter::convertInstrument(*instrument));
}

void NotationParts::updatePartTitles()
{
    for (const Part* part: score()->parts()) {
        setPartName(part->id(), formatPartName(part));
    }
}

void NotationParts::doMovePart(const ID& sourcePartId, const ID& destinationPartId, INotationParts::InsertMode mode)
{
    Part* part = this->part(sourcePartId);
    Part* destinationPart = this->part(destinationPartId);
    if (!part || !destinationPart) {
        return;
    }

    bool partIsBefore = score()->staffIdx(part) < score()->staffIdx(destinationPart);

    std::vector<Staff*> staves;
    for (Staff* staff: *part->staves()) {
        staves.push_back(staff);
    }

    int destinationStaffIndex = partIsBefore ? static_cast<int>(staves.size()) : 0;

    score()->undoRemovePart(part);

    int toPartIndex = score()->parts().indexOf(destinationPart);
    int newPartIndex = mode == InsertMode::Before ? toPartIndex : toPartIndex + 1;
    score()->parts().insert(newPartIndex, part);

    auto instruments = *part->instruments();
    doMoveStaves(staves, destinationStaffIndex);
    part->setInstruments(instruments);
}

void NotationParts::doMoveStaves(const std::vector<Staff*>& staves, int destinationStaffIndex, Part* destinationPart)
{
    for (Staff* staff: staves) {
        Staff* movedStaff = staff->clone();

        if (destinationPart) {
            movedStaff->setPart(destinationPart);
        }

        bool needUnlink = !staff->isLinked();

        insertStaff(movedStaff, destinationStaffIndex);
        Ms::Excerpt::cloneStaff(staff, movedStaff);

        if (needUnlink) {
            movedStaff->undoUnlink();
        }

        ++destinationStaffIndex;
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

    score()->undo(new Ms::ChangeInstrumentLong(instrumentInfo.fraction, part, { StaffName(name, 0) }));
    updateScore();

    m_partsNotifier->itemChanged(part);
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

    score()->undo(new Ms::ChangeInstrumentShort(instrumentInfo.fraction, part, { StaffName(abbreviature, 0) }));
    updateScore();

    m_partsNotifier->itemChanged(part);
}

void NotationParts::setStaffVisible(const ID& staffId, bool visible)
{
    Staff* staff = this->staff(staffId);
    if (!staff) {
        return;
    }

    if (staff->show() == visible) {
        return;
    }

    doSetStaffVisible(staff, visible);
    updateScore();

    notifyAboutStaffChanged(staffId);
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

    score()->undo(new Ms::ChangeStaffType(staff, *staffType));
    updateScore();

    notifyAboutStaffChanged(staffId);
}

void NotationParts::setCutawayEnabled(const ID& staffId, bool enabled)
{
    Staff* staff = this->staff(staffId);
    if (!staff) {
        return;
    }

    staff->setCutaway(enabled);
    score()->undo(new Ms::ChangeStaff(staff));
    updateScore();

    notifyAboutStaffChanged(staffId);
}

void NotationParts::setSmallStaff(const ID& staffId, bool smallStaff)
{
    Staff* staff = this->staff(staffId);
    if (!staff) {
        return;
    }

    Ms::StaffType* staffType = staff->staffType(DEFAULT_TICK);
    if (!staffType) {
        return;
    }

    staffType->setSmall(smallStaff);
    score()->undo(new Ms::ChangeStaffType(staff, *staffType));
    updateScore();

    notifyAboutStaffChanged(staffId);
}

void NotationParts::setStaffConfig(const ID& staffId, const StaffConfig& config)
{
    Staff* staff = this->staff(staffId);
    if (!staff) {
        return;
    }

    Ms::StaffType* staffType = staff->staffType(DEFAULT_TICK);
    if (!staffType) {
        return;
    }

    staff->setVisible(config.visible);
    staff->undoChangeProperty(Ms::Pid::COLOR, config.linesColor);
    staff->setInvisible(config.visibleLines);
    staff->setUserDist(config.userDistance);
    staff->undoChangeProperty(Ms::Pid::MAG, config.scale);
    staff->setShowIfEmpty(config.showIfEmpty);
    staffType->setLines(config.linesCount);
    staffType->setLineDistance(Ms::Spatium(config.lineDistance));
    staffType->setGenClef(config.showClef);
    staffType->setGenTimesig(config.showTimeSignature);
    staffType->setGenKeysig(config.showKeySignature);
    staffType->setShowBarlines(config.showBarlines);
    staffType->setStemless(config.showStemless);
    staffType->setShowLedgerLines(config.showLedgerLinesPitched);
    staffType->setNoteHeadScheme(config.noteheadScheme);
    staff->setHideSystemBarLine(config.hideSystemBarline);
    staff->setMergeMatchingRests(config.mergeMatchingRests);
    staff->setHideWhenEmpty(config.hideMode);
    staff->setDefaultClefType(config.clefType);

    score()->undo(new Ms::ChangeStaff(staff));
    updateScore();

    notifyAboutStaffChanged(staffId);
}

bool NotationParts::voiceVisible(int voiceIndex) const
{
    if (!score()) {
        return false;
    }

    for (const Part* part : score()->parts()) {
        for (Staff* staff : *part->staves()) {
            if (staff->isVoiceVisible(voiceIndex)) {
                return true;
            }
        }
    }

    return false;
}

void NotationParts::setVoiceVisible(int voiceIndex, bool visible)
{
    if (voiceVisible(voiceIndex) == visible) {
        return;
    }

    for (const Part* part : score()->parts()) {
        for (Staff* staff : *part->staves()) {
            doSetStaffVoiceVisible(staff, voiceIndex, visible);
        }
    }

    updateScore();
}

void NotationParts::setVoiceVisible(const ID& staffId, int voiceIndex, bool visible)
{
    Staff* staff = this->staff(staffId);
    if (!staff) {
        return;
    }

    doSetStaffVoiceVisible(staff, voiceIndex, visible);
    updateScore();

    notifyAboutStaffChanged(staffId);
}

void NotationParts::doSetStaffVoiceVisible(Staff* staff, int voiceIndex, bool visible)
{
    if (staff->isVoiceVisible(voiceIndex) == visible) {
        return;
    }

    static const QSet<Ms::ElementType> ignoredTypes {
        Ms::ElementType::STAFF_LINES,
        Ms::ElementType::BAR_LINE,
        Ms::ElementType::BRACKET,
        Ms::ElementType::TIMESIG,
        Ms::ElementType::CLEF
    };

    for (Ms::Page* page : score()->pages()) {
        for (Ms::Element* element : page->elements()) {
            if (!element) {
                continue;
            }

            if (element->staffIdx() != staff->idx() || element->voice() != voiceIndex) {
                continue;
            }

            if (ignoredTypes.contains(element->type())) {
                continue;
            }

            element->undoChangeProperty(Ms::Pid::VISIBLE, visible);
        }
    }

    staff->setVoiceVisible(voiceIndex, visible);
}

void NotationParts::appendDoublingInstrument(const mu::instruments::Instrument& instrument, const ID& destinationPartId)
{
    Part* part = this->part(destinationPartId);
    if (!part) {
        return;
    }

    int lastTick = 1;
    for (const Ms::Fraction& fraction: instruments(part).keys()) {
        lastTick = std::max(fraction.ticks(), lastTick);
    }

    part->setInstrument(InstrumentsConverter::convertInstrument(instrument), Ms::Fraction::fromTicks(lastTick + 1));
    doSetPartName(part, formatPartName(part));
    updateScore();

    ChangedNotifier<mu::instruments::Instrument>* notifier = partNotifier(destinationPartId);
    notifier->itemAdded(instrument);
    m_partsNotifier->itemChanged(part);
}

void NotationParts::appendStaff(Staff* staff, const ID& destinationPartId)
{
    Part* destinationPart = part(destinationPartId);
    if (!destinationPart) {
        return;
    }

    InstrumentInfo instrumentInfo = this->instrumentInfo(destinationPart->instrumentId(), destinationPart);
    if (!instrumentInfo.isValid()) {
        return;
    }

    int staffIndex = destinationPart->nstaves();

    staff->setScore(score());
    staff->setPart(destinationPart);

    insertStaff(staff, staffIndex);
    updateScore();

    Ms::Instrument* instrument = instrumentInfo.instrument;
    instrument->setClefType(staffIndex, staff->defaultClefType());

    ChangedNotifier<const Staff*>* notifier = instrumentNotifier(instrument->instrumentId(), destinationPartId);
    notifier->itemAdded(staff);
}

void NotationParts::cloneStaff(const ID& sourceStaffId, const ID& destinationStaffId)
{
    Staff* sourceStaff = staff(sourceStaffId);
    Staff* destinationStaff = staff(destinationStaffId);

    if (!sourceStaff || !destinationStaff) {
        return;
    }

    Ms::Excerpt::cloneStaff(sourceStaff, destinationStaff);
    updateScore();
}

void NotationParts::replaceInstrument(const ID& instrumentId, const ID& fromPartId, const mu::instruments::Instrument& newInstrument)
{
    Part* part = this->part(fromPartId);
    if (!part) {
        return;
    }

    InstrumentInfo oldInstrumentInfo = this->instrumentInfo(instrumentId, part);
    if (!oldInstrumentInfo.isValid()) {
        return;
    }

    part->setInstrument(InstrumentsConverter::convertInstrument(newInstrument), oldInstrumentInfo.fraction);
    doSetPartName(part, formatPartName(part));
    updateScore();

    ChangedNotifier<mu::instruments::Instrument>* notifier = partNotifier(part->id());
    notifier->itemReplaced(InstrumentsConverter::convertInstrument(*oldInstrumentInfo.instrument), newInstrument);

    m_partsNotifier->itemChanged(part);
}

Notification NotationParts::partsChanged() const
{
    return m_partsChanged;
}

INotationUndoStackPtr NotationParts::undoStack() const
{
    return m_undoStack;
}

void NotationParts::removeParts(const IDList& partsIds)
{
    if (partsIds.empty()) {
        return;
    }

    doRemoveParts(partsIds);
    updateScore();
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

    doRemoveInstruments(instrumentIds, part);
    doSetPartName(part, formatPartName(part));
    updateScore();

    m_partsNotifier->itemChanged(part);
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

    for (Staff* staff: staves(stavesIds)) {
        score()->cmdRemoveStaff(staff->idx());
    }

    updateScore();
}

void NotationParts::doSetPartName(Part* part, const QString& name)
{
    score()->undo(new Ms::ChangePart(part, new Ms::Instrument(*part->instrument()), name));
}

void NotationParts::insertStaff(Staff* staff, int destinationStaffIndex)
{
    if (score()->excerpt()) {
        int globalDestinationStaffIndex = score()->staffIdx(staff->part()) + destinationStaffIndex;

        for (int voiceIndex = 0; voiceIndex < VOICES; ++voiceIndex) {
            int track = globalDestinationStaffIndex * VOICES + voiceIndex % VOICES;
            score()->excerpt()->tracks().insert(track, track);
        }
    }

    score()->undoInsertStaff(staff, destinationStaffIndex);
}

void NotationParts::moveParts(const IDList& sourcePartsIds, const ID& destinationPartId, InsertMode mode)
{
    for (const ID& sourcePartId: sourcePartsIds) {
        doMovePart(sourcePartId, destinationPartId, mode);
    }

    updateScore();
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

    doRemoveInstruments(sourceInstrumentsIds, fromPart);
    doInsertInstruments(movingInstruments, destinationPartId, destinationInstrumentId, mode);

    doSetPartName(fromPart, formatPartName(fromPart));
    if (fromPart != toPart) {
        doSetPartName(toPart, formatPartName(toPart));
    }
    updateScore();

    m_partsNotifier->itemChanged(fromPart);
    notifyAboutInstrumentsChanged(fromPart->id());
    if (fromPart != toPart) {
        notifyAboutInstrumentsChanged(toPart->id());
        m_partsNotifier->itemChanged(toPart);
    }
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

    int newInstrumentIndex = (mode == InsertMode::Before ? destinationIndex : destinationIndex + 1);

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

    Staff* destinationStaff = staff(destinationStaffId);
    if (!destinationStaff) {
        return;
    }

    std::vector<Staff*> staves = this->staves(sourceStavesIds);
    Part* destinationPart = destinationStaff->part();
    int destinationStaffIndex = (mode == InsertMode::Before ? destinationStaff->idx() : destinationStaff->idx() + 1);
    destinationStaffIndex -= score()->staffIdx(destinationPart); // NOTE: convert to local part's staff index

    doMoveStaves(staves, destinationStaffIndex, destinationPart);
    updateScore();
}

mu::ValCh<bool> NotationParts::canChangeInstrumentVisibility(const ID& instrumentId, const ID& fromPartId) const
{
    InstrumentKey key { fromPartId, instrumentId };

    if (!m_canChangeInstrumentsVisibilityHash.contains(key)) {
        m_canChangeInstrumentsVisibilityHash[key].val = resolveCanChangeInstrumentVisibility(fromPartId, instrumentId);
    }

    return m_canChangeInstrumentsVisibilityHash[key];
}

std::vector<Part*> NotationParts::availableParts(const Ms::Score* score) const
{
    std::vector<Part*> parts;

    if (!score) {
        return parts;
    }

    std::vector<Part*> scoreParts = this->scoreParts(score);
    parts.insert(parts.end(), scoreParts.begin(), scoreParts.end());

    std::vector<Part*> excerptParts = this->excerptParts(score);
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

    std::vector<Part*> parts = availableParts(score);

    for (Part* part: parts) {
        if (part->id() == partId) {
            return part;
        }
    }

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
    return score()->staff(staffId);
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
    Part* partCopy = new Part(*part);
    partCopy->staves()->clear();

    int partIndex = resolvePartIndex(part);
    score()->parts().insert(partIndex, partCopy);

    if (score()->excerpt()) {
        score()->excerpt()->parts().append(part);
    }

    for (int staffIndex = 0; staffIndex < part->nstaves(); ++staffIndex) {
        Staff* staff = part->staff(staffIndex);

        Staff* staffCopy = new Staff(score());
        staffCopy->setId(staff->id());
        staffCopy->setPart(partCopy);
        staffCopy->init(staff);

        insertStaff(staffCopy, staffIndex);

        Ms::Fraction startTick = score()->firstMeasure()->tick();
        Ms::Fraction endTick = score()->lastMeasure()->tick();
        Ms::Excerpt::cloneStaff2(staff, staffCopy, startTick, endTick);
    }

    partCopy->setScore(score());

    updateScore();

    m_partsNotifier->itemChanged(part);
}

int NotationParts::resolvePartIndex(Part* part) const
{
    auto findMasterPartIndex = [this](const ID& partId) -> int {
        QList<Part*> masterParts = masterScore()->parts();

        for (int masterPartIndex = 0; masterPartIndex < masterParts.size(); ++masterPartIndex) {
            if (masterParts[masterPartIndex]->id() == partId) {
                return masterPartIndex;
            }
        }

        return -1;
    };

    const QList<Part*>& scoreParts = score()->parts();

    int originPartIndex = findMasterPartIndex(part->id());
    Part* destinationPart = nullptr;

    for (Part* scorePart : scoreParts) {
        int masterPartIndex = findMasterPartIndex(scorePart->id());

        if (masterPartIndex < originPartIndex) {
            continue;
        }

        destinationPart = scorePart;
        break;
    }

    if (destinationPart) {
        return scoreParts.indexOf(destinationPart);
    }

    return scoreParts.size();
}

void NotationParts::appendStaves(Part* part, const mu::instruments::Instrument& instrument)
{
    for (int staffIndex = 0; staffIndex < instrument.staves; ++staffIndex) {
        int lastStaffIndex = this->lastStaffIndex();

        Staff* staff = new Staff(score());
        staff->setPart(part);
        initStaff(staff, instrument, Ms::StaffType::preset(StaffType::STANDARD), staffIndex);

        if (lastStaffIndex > 0) {
            staff->setBarLineSpan(score()->staff(lastStaffIndex - 1)->barLineSpan());
        }

        insertStaff(staff, staffIndex);
    }
}

void NotationParts::removeMissingInstruments(const InstrumentList& instruments)
{
    IDList partsToRemove;
    IDList instrumentIds;
    for (const mu::instruments::Instrument& instrument : instruments) {
        instrumentIds << instrument.id;
    }

    for (const Part* part: partList()) {
        auto partInstruments = this->instruments(part);
        IDList instrumentsToRemove;

        for (const Ms::Instrument* instrument: partInstruments.values()) {
            if (!instrumentIds.contains(instrument->instrumentId())) {
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

void NotationParts::appendNewInstruments(const InstrumentList& instruments)
{
    IDList instrumentIds;
    for (const mu::instruments::Instrument& instrument : instruments) {
        instrumentIds << instrument.id;
    }

    IDList existedInstrumentIds = allInstrumentsIds();
    IDList newInstruentIds = instrumentIds;
    for (const ID& instrumentId: existedInstrumentIds) {
        newInstruentIds.removeOne(instrumentId);
    }

    for (const mu::instruments::Instrument& instrument: instruments) {
        if (!newInstruentIds.contains(instrument.id)) {
            continue;
        }

        newInstruentIds.removeOne(instrument.id);

        Part* part = new Part(score());

        part->setPartName(instrument.name);
        part->setInstrument(InstrumentsConverter::convertInstrument(instrument));

        score()->undo(new Ms::InsertPart(part, lastStaffIndex()));
        appendStaves(part, instrument);
    }
}

void NotationParts::sortParts(const InstrumentList& instruments)
{
    Q_ASSERT(score()->parts().size() == static_cast<int>(instruments.size()));

    auto mainInstrumentId = [](const Part* part) {
        return part->instrument()->instrumentId();
    };

    for (int i = 0; i < instruments.size(); ++i) {
        const Part* currentPart = score()->parts().at(i);

        if (mainInstrumentId(currentPart) == instruments.at(i).id) {
            continue;
        }

        for (int j = i; j < score()->parts().size(); ++j) {
            const Part* part = score()->parts().at(j);

            if (mainInstrumentId(part) == instruments.at(i).id) {
                doMovePart(part->id(), currentPart->id());
                break;
            }
        }
    }
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

void NotationParts::initStaff(Staff* staff, const mu::instruments::Instrument& instrument, const Ms::StaffType* staffType, int cleffIndex)
{
    const Ms::StaffType* staffTypePreset = staffType ? staffType : instrument.staffTypePreset;
    if (!staffTypePreset) {
        staffTypePreset = Ms::StaffType::getDefaultPreset(instrument.staffGroup);
    }

    Ms::StaffType* stt = staff->setStaffType(DEFAULT_TICK, *staffTypePreset);
    if (cleffIndex >= mu::instruments::MAX_STAVES) {
        stt->setSmall(false);
    } else {
        stt->setSmall(instrument.smallStaff[cleffIndex]);
        staff->setBracketType(0, instrument.bracket[cleffIndex]);
        staff->setBracketSpan(0, instrument.bracketSpan[cleffIndex]);
        staff->setBarLineSpan(instrument.barlineSpan[cleffIndex]);
    }
    staff->setDefaultClefType(instrument.clefs[cleffIndex]);
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
    ChangedNotifier<mu::instruments::Instrument>* notifier = partNotifier(partId);
    for (const mu::instruments::Instrument& instrument: instruments) {
        notifier->itemChanged(instrument);
    }
}

ChangedNotifier<mu::instruments::Instrument>* NotationParts::partNotifier(const ID& partId) const
{
    if (m_partsNotifiersMap.find(partId) != m_partsNotifiersMap.end()) {
        return m_partsNotifiersMap[partId];
    }

    ChangedNotifier<mu::instruments::Instrument>* notifier = new ChangedNotifier<mu::instruments::Instrument>();
    auto value = std::pair<ID, ChangedNotifier<mu::instruments::Instrument>*>(partId, notifier);
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
