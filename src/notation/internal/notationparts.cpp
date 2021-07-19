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

#include "libmscore/score.h"
#include "libmscore/undo.h"
#include "libmscore/excerpt.h"
#include "libmscore/drumset.h"
#include "libmscore/instrchange.h"
#include "libmscore/page.h"

#include "instrumentsconverter.h"
#include "scoreorderconverter.h"

#include "igetscore.h"

#include "log.h"

using namespace mu::async;
using namespace mu::notation;

static const Ms::Fraction DEFAULT_TICK = Ms::Fraction(0, 1);

static QString formatInstrumentName(const QString& instrumentName, const Trait& trait, int instrumentNumber)
{
    QString numberPart = instrumentNumber > 0 ? " " + QString::number(instrumentNumber) : QString();

    if (trait.type != TraitType::Transposition || trait.isHiddenOnScore) {
        return instrumentName + numberPart;
    }

    return qtrc("instruments", "%1 in %2%3").arg(instrumentName).arg(trait.name).arg(numberPart);
}

NotationParts::NotationParts(IGetScore* getScore, INotationInteractionPtr interaction, INotationUndoStackPtr undoStack)
    : m_getScore(getScore), m_undoStack(undoStack), m_partsNotifier(new ChangedNotifier<const Part*>())
{
    interaction->selectionChanged().onNotify(this, [this]() {
        updateCanChangeInstrumentsVisibility();
    });

    interaction->dropChanged().onNotify(this, [this]() {
        updatePartTitles();
    });

    m_undoStack->undoNotification().onNotify(this, [this]() {
        m_partsNotifier->changed();
    });

    m_undoStack->redoNotification().onNotify(this, [this]() {
        m_partsNotifier->changed();
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

NotifyList<Instrument> NotationParts::instrumentList(const ID& partId) const
{
    Part* part = this->part(partId);
    if (!part) {
        return NotifyList<Instrument>();
    }

    NotifyList<Instrument> result;

    for (const Ms::Instrument* instrument: instruments(part).values()) {
        result.push_back(InstrumentsConverter::convertInstrument(*instrument));
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

void NotationParts::setParts(const PartInstrumentList& parts)
{
    TRACEFUNC;

    QList<Ms::Staff*> originalStaves = masterScore()->staves();

    removeMissingParts(parts);
    appendNewParts(parts);
    updateSoloist(parts);

    sortParts(parts, masterScore(), originalStaves);

    masterScore()->setBracketsAndBarlines();

    updateScore();

    m_partsNotifier->changed();
}

void NotationParts::setScoreOrder(const ScoreOrder& order)
{
    TRACEFUNC;

    Ms::ScoreOrder so = ScoreOrderConverter::convertScoreOrder(order);
    masterScore()->undo(new Ms::ChangeScoreOrder(masterScore(), so));
    masterScore()->setBracketsAndBarlines();
}

void NotationParts::setPartVisible(const ID& partId, bool visible)
{
    TRACEFUNC;

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

    notifyAboutPartChanged(partId);
}

void NotationParts::setPartName(const ID& partId, const QString& name)
{
    TRACEFUNC;

    Part* part = this->part(partId);
    if (!part || part->partName() == name) {
        return;
    }

    doSetPartName(part, name);
    updateScore();

    notifyAboutPartChanged(partId);
}

void NotationParts::setPartSharpFlat(const ID& partId, const SharpFlat& sharpFlat)
{
    TRACEFUNC;

    Part* part = this->part(partId);
    if (!part) {
        return;
    }

    part->undoChangeProperty(Ms::Pid::PREFER_SHARP_FLAT, static_cast<int>(sharpFlat));
    updateScore();

    notifyAboutPartChanged(partId);
}

void NotationParts::setPartTransposition(const ID& partId, const Interval& transpose)
{
    TRACEFUNC;

    Part* part = this->part(partId);
    if (!part) {
        return;
    }

    score()->transpositionChanged(part, transpose);
    updateScore();

    notifyAboutPartChanged(partId);
}

void NotationParts::setInstrumentVisible(const ID& instrumentId, const ID& fromPartId, bool visible)
{
    TRACEFUNC;

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

    ChangedNotifier<Instrument>* notifier = partNotifier(fromPartId);
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
        if (instrumentChange->instrument()->getId() == instrumentId) {
            return false;
        }
    }

    return true;
}

void NotationParts::assignIstrumentToSelectedChord(Ms::Instrument* instrument)
{
    TRACEFUNC;

    Ms::ChordRest* chord = selectedChord();
    if (!chord) {
        return;
    }

    Part* part = chord->part();
    part->removeInstrument(instrument->getId());
    part->setInstrument(instrument, chord->segment()->tick());

    auto instrumentChange = new Ms::InstrumentChange(*instrument, score());
    instrumentChange->setInit(true);
    instrumentChange->setParent(chord->segment());
    instrumentChange->setTrack((chord->track() / VOICES) * VOICES);
    instrumentChange->setupInstrument(instrument);

    score()->undoAddElement(instrumentChange);
    updateScore();

    ChangedNotifier<Instrument>* notifier = partNotifier(part->id());
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
    TRACEFUNC;

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
    TRACEFUNC;

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
    TRACEFUNC;

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

    notifyAboutPartChanged(fromPartId);
}

void NotationParts::setInstrumentAbbreviature(const ID& instrumentId, const ID& fromPartId, const QString& abbreviature)
{
    TRACEFUNC;

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

    notifyAboutPartChanged(fromPartId);
}

void NotationParts::setStaffVisible(const ID& staffId, bool visible)
{
    TRACEFUNC;

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
    TRACEFUNC;

    if (!staff) {
        return;
    }

    staff->setInvisible(Fraction(0, 1), !visible);
    score()->undo(new Ms::ChangeStaff(staff));
}

void NotationParts::setStaffType(const ID& staffId, StaffType type)
{
    TRACEFUNC;

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
    TRACEFUNC;

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
    TRACEFUNC;

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
    TRACEFUNC;

    Staff* staff = this->staff(staffId);
    if (!staff) {
        return;
    }

    Ms::StaffType* staffType = staff->staffType(DEFAULT_TICK);
    if (!staffType) {
        return;
    }

    staff->setVisible(config.visible);
    staff->undoChangeProperty(Ms::Pid::STAFF_COLOR, config.linesColor);
    staff->setInvisible(Fraction(0, 1), config.visibleLines);
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
    staff->setDefaultClefType(config.clefTypeList);
    staff->setCutaway(config.cutaway);
    staff->undoChangeProperty(Ms::Pid::SMALL, config.small);

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
    TRACEFUNC;

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
    TRACEFUNC;

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
    TRACEFUNC;

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

void NotationParts::appendDoublingInstrument(const Instrument& instrument, const ID& destinationPartId)
{
    TRACEFUNC;

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

    ChangedNotifier<Instrument>* notifier = partNotifier(destinationPartId);
    notifier->itemAdded(instrument);
    notifyAboutPartChanged(destinationPartId);
}

void NotationParts::appendStaff(Staff* staff, const ID& destinationPartId)
{
    TRACEFUNC;

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

    masterScore()->setBracketsAndBarlines();

    updateScore();

    Ms::Instrument* instrument = instrumentInfo.instrument;
    instrument->setClefType(staffIndex, staff->defaultClefType());

    ChangedNotifier<const Staff*>* notifier = instrumentNotifier(instrument->getId(), destinationPartId);
    notifier->itemAdded(staff);
}

void NotationParts::cloneStaff(const ID& sourceStaffId, const ID& destinationStaffId)
{
    TRACEFUNC;

    Staff* sourceStaff = staff(sourceStaffId);
    Staff* destinationStaff = staff(destinationStaffId);

    if (!sourceStaff || !destinationStaff) {
        return;
    }

    Ms::Excerpt::cloneStaff(sourceStaff, destinationStaff);
    updateScore();
}

void NotationParts::replaceInstrument(const ID& instrumentId, const ID& fromPartId, const Instrument& newInstrument)
{
    TRACEFUNC;

    Part* part = this->part(fromPartId);
    if (!part) {
        return;
    }

    InstrumentInfo oldInstrumentInfo = this->instrumentInfo(instrumentId, part);
    if (!oldInstrumentInfo.isValid()) {
        return;
    }

    score()->undo(new Ms::ChangePart(part, new Ms::Instrument(InstrumentsConverter::convertInstrument(newInstrument)),
                                     formatPartName(part)));
    updateScore();

    ChangedNotifier<Instrument>* notifier = partNotifier(part->id());
    notifier->itemReplaced(InstrumentsConverter::convertInstrument(*oldInstrumentInfo.instrument), newInstrument);

    notifyAboutPartChanged(fromPartId);
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
    TRACEFUNC;

    if (partsIds.empty()) {
        return;
    }

    PartInstrumentList parts;
    for (Ms::Part* currentPart: masterScore()->parts()) {
        if (partsIds.contains(currentPart->id())) {
            continue;
        }
        PartInstrument pi;
        pi.isExistingPart = true;
        pi.partId = currentPart->id();
        parts << pi;
    }

    QList<Ms::Staff*> originalStaves = masterScore()->staves();

    removeMissingParts(parts);

    sortParts(parts, masterScore(), originalStaves);

    masterScore()->setBracketsAndBarlines();

    updateScore();
}

void NotationParts::doRemoveParts(const IDList& partsIds)
{
    TRACEFUNC;

    for (const ID& partId: partsIds) {
        for (Ms::Excerpt* currentExcerpt: masterScore()->excerpts()) {
            for (Ms::Part* currentPart: currentExcerpt->parts()) {
                if (currentPart->id() == partId) {
                    currentExcerpt->removePart(partId);
                    break;
                }
            }
        }

        Part* p = part(partId);
        m_partsNotifier->itemRemoved(p);
        score()->cmdRemovePart(p);
    }
}

void NotationParts::removeInstruments(const IDList& instrumentIds, const ID& fromPartId)
{
    TRACEFUNC;

    Part* part = this->part(fromPartId);
    if (!part) {
        return;
    }

    doRemoveInstruments(instrumentIds, part);
    doSetPartName(part, formatPartName(part));
    updateScore();

    notifyAboutPartChanged(fromPartId);
}

void NotationParts::doRemoveInstruments(const IDList& instrumentIds, Part* fromPart)
{
    TRACEFUNC;

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
    TRACEFUNC;

    if (stavesIds.empty()) {
        return;
    }

    for (Staff* staff: staves(stavesIds)) {
        score()->cmdRemoveStaff(staff->idx());
    }

    masterScore()->setBracketsAndBarlines();
    updateScore();
}

void NotationParts::doSetPartName(Part* part, const QString& name)
{
    TRACEFUNC;

    score()->undo(new Ms::ChangePart(part, new Ms::Instrument(*part->instrument()), name));
}

void NotationParts::insertStaff(Staff* staff, int destinationStaffIndex)
{
    TRACEFUNC;

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
    TRACEFUNC;

    IDList partIds;

    for (Ms::Part* currentPart: masterScore()->parts()) {
        partIds << currentPart->id();
    }

    for (const ID& sourcePartId: sourcePartsIds) {
        int srcIndex = partIds.indexOf(sourcePartId);
        int dstIndex = partIds.indexOf(destinationPartId);
        dstIndex += (mode == InsertMode::Before) && (srcIndex < dstIndex) ? -1 : 0;
        partIds.move(srcIndex, dstIndex);
    }

    PartInstrumentList parts;
    for (ID& partId: partIds) {
        PartInstrument pi;
        pi.isExistingPart = true;
        pi.partId = partId;
        parts << pi;
    }

    sortParts(parts, masterScore(), masterScore()->staves());

    masterScore()->setBracketsAndBarlines();

    updateScore();

    m_partsNotifier->changed();
}

void NotationParts::moveInstruments(const IDList& sourceInstrumentsIds, const ID& sourcePartId, const ID& destinationPartId,
                                    const ID& destinationInstrumentId, InsertMode mode)
{
    TRACEFUNC;

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

    notifyAboutPartChanged(sourcePartId);
    notifyAboutInstrumentsChanged(fromPart->id());
    if (fromPart != toPart) {
        notifyAboutInstrumentsChanged(toPart->id());
        notifyAboutPartChanged(destinationPartId);
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

        bool acceptedByFilter = !filterInstrumentsIds.isEmpty() ? filterInstrumentsIds.contains(instrument->getId()) : true;
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
    TRACEFUNC;

    Part* destinationPart = part(destinationPartId);
    if (!destinationPart) {
        return;
    }

    QMap<Ms::Fraction, Ms::Instrument*> partInstrumentsMap = this->instruments(destinationPart);
    QList<Ms::Fraction> partInstrumentsFractions = partInstrumentsMap.keys();
    QList<Ms::Instrument*> partInstruments = partInstrumentsMap.values();

    int destinationIndex = 0;
    for (int i = 0; i < partInstruments.size(); i++) {
        if (partInstruments[i]->getId() == destinationInstrumentId) {
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
    TRACEFUNC;

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

    masterScore()->setBracketsAndBarlines();

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
        if (instrument->getId() == instrumentId) {
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
    TRACEFUNC;

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

void NotationParts::appendStaves(Part* part, const Instrument& instrument)
{
    TRACEFUNC;

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

    if (!part->nstaves()) {
        return;
    }
    int firstStaffIndex = part->staff(0)->idx();
    int endStaffIndex = firstStaffIndex + part->nstaves();
    masterScore()->adjustKeySigs(firstStaffIndex, endStaffIndex, masterScore()->keyList());
}

void NotationParts::removeMissingParts(const PartInstrumentList& parts)
{
    TRACEFUNC;

    IDList partsToRemove;

    IDList partIds;
    for (const PartInstrument& pi: parts) {
        if (pi.isExistingPart) {
            partIds << pi.partId;
        }
    }

    for (const Part* part: partList()) {
        if (partIds.contains(part->id())) {
            continue;
        }
        partsToRemove << part->id();
    }

    doRemoveParts(partsToRemove);
}

void NotationParts::appendNewParts(const PartInstrumentList& parts)
{
    TRACEFUNC;

    Instruments newInstruments;

    for (const PartInstrument& pi: parts) {
        newInstruments << pi.instrument;
    }

    int staffCount = 0;
    for (const PartInstrument& pi: parts) {
        if (pi.isExistingPart) {
            staffCount += part(pi.partId)->nstaves();
            continue;
        }

        Part* part = new Part(score());
        const Instrument& instrument = pi.instrument;

        part->setPartName(instrument.name);
        part->setSoloist(pi.isSoloist);
        part->setInstrument(InstrumentsConverter::convertInstrument(instrument));

        int instrumentNumber = resolveInstrumentNumber(newInstruments, instrument);

        QString longName = !instrument.longNames.empty() ? instrument.longNames.first().name() : QString();
        QString formattedLongName = formatInstrumentName(longName, instrument.trait, instrumentNumber);
        QString shortName = !instrument.shortNames.empty() ? instrument.shortNames.first().name() : QString();
        QString formattedShortName = formatInstrumentName(shortName, instrument.trait, instrumentNumber);

        part->setLongName(formattedLongName);
        part->setShortName(formattedShortName);

        score()->undo(new Ms::InsertPart(part, staffCount));
        appendStaves(part, pi.instrument);
        staffCount += part->nstaves();

        m_partsNotifier->itemAdded(part);
    }
}

void NotationParts::updateSoloist(const PartInstrumentList& parts)
{
    TRACEFUNC;

    for (const PartInstrument& pi: parts) {
        if (pi.isExistingPart && (pi.isSoloist != part(pi.partId)->soloist())) {
            score()->undo(new Ms::SetSoloist(part(pi.partId), pi.isSoloist));
        }
    }
}

void NotationParts::sortParts(const PartInstrumentList& parts, const Ms::Score* score, const QList<Ms::Staff*>& originalStaves)
{
    TRACEFUNC;

    QList<int> staffMapping;
    QList<int> trackMapping;
    int runningStaffIndex = 0;
    bool sortingNeeded = false;

    int partIndex = 0;
    for (const PartInstrument& pi: parts) {
        Ms::Part* currentPart = pi.isExistingPart ? part(pi.partId) : score->parts()[partIndex];
        for (Ms::Staff* staff: *currentPart->staves()) {
            int actualStaffIndex = score->staves().indexOf(staff);

            trackMapping.append(originalStaves.indexOf(staff));
            staffMapping.append(actualStaffIndex);
            sortingNeeded |= actualStaffIndex != runningStaffIndex;
            ++runningStaffIndex;
        }
        ++partIndex;
    }

    if (sortingNeeded) {
        score->masterScore()->undo(new Ms::SortStaves(score->masterScore(), staffMapping));
    }
    score->masterScore()->undo(new Ms::MapExcerptTracks(score->masterScore(), trackMapping));
}

int NotationParts::resolveInstrumentNumber(const Instruments& newInstruments,
                                           const Instrument& currentInstrument) const
{
    int count = 0;

    for (const Part* part : score()->parts()) {
        const Ms::Instrument* partInstrument = part->instrument();

        if (partInstrument->getId() == currentInstrument.id
            && partInstrument->trait().name == currentInstrument.trait.name) {
            ++count;
        }
    }

    if (count > 0) {
        return count + 1;
    }

    for (const Instrument& newInstrument: newInstruments) {
        if (newInstrument.id == currentInstrument.id
            && newInstrument.trait.name == currentInstrument.trait.name) {
            ++count;
        }
    }

    return count > 1 ? 1 : 0;
}

IDList NotationParts::allInstrumentsIds() const
{
    IDList result;

    for (const Part* part: partList()) {
        auto partInstruments = instruments(part);

        for (const Ms::Instrument* instrument: partInstruments.values()) {
            result << instrument->getId();
        }
    }

    return result;
}

int NotationParts::lastStaffIndex() const
{
    return !score()->staves().isEmpty() ? score()->staves().last()->idx() : 0;
}

void NotationParts::initStaff(Staff* staff, const Instrument& instrument, const Ms::StaffType* staffType, int cleffIndex)
{
    TRACEFUNC;

    const Ms::StaffType* staffTypePreset = staffType ? staffType : instrument.staffTypePreset;
    if (!staffTypePreset) {
        staffTypePreset = Ms::StaffType::getDefaultPreset(instrument.staffGroup);
    }

    Ms::StaffType* stt = staff->setStaffType(DEFAULT_TICK, *staffTypePreset);
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

void NotationParts::notifyAboutPartChanged(const ID& partId) const
{
    Part* part = this->part(partId);
    if (!part) {
        return;
    }

    m_partsNotifier->itemChanged(part);
}

void NotationParts::notifyAboutStaffChanged(const ID& staffId) const
{
    Staff* staff = this->staff(staffId);
    if (!staff) {
        return;
    }

    InstrumentInfo instrumentInfo = this->instrumentInfo(staff);
    ChangedNotifier<const Staff*>* notifier = instrumentNotifier(instrumentInfo.instrument->getId(), staff->part()->id());
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
