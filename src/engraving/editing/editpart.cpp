/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "editpart.h"

#include <algorithm>
#include <map>
#include <unordered_set>

#include "containers.h"
#include "translation.h"

#include "addremoveelement.h"
#include "editexcerpt.h"
#include "editpagelocks.h"
#include "editscoreproperties.h"
#include "editstaff.h"
#include "editsystemlocks.h"
#include "transaction/transaction.h"
#include "transpose.h"

#include "../dom/excerpt.h"
#include "../dom/factory.h"
#include "../dom/instrchange.h"
#include "../dom/instrtemplate.h"
#include "../dom/scoreorder.h"
#include "../dom/masterscore.h"
#include "../dom/mscore.h"
#include "../dom/part.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/staff.h"
#include "../dom/stafftype.h"
#include "../dom/stringtunings.h"
#include "../dom/utils.h"

#include "log.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   InsertPart
//---------------------------------------------------------

InsertPart::InsertPart(Part* p, size_t targetPartIdx)
{
    m_part = p;
    m_targetPartIdx = targetPartIdx;
}

void InsertPart::undo()
{
    m_part->score()->removePart(m_part);
}

void InsertPart::redo()
{
    m_part->score()->insertPart(m_part, m_targetPartIdx);
}

void InsertPart::cleanup(bool wasDone)
{
    if (!wasDone) {
        delete m_part;
        m_part = nullptr;
    }
}

//---------------------------------------------------------
//   RemovePart
//---------------------------------------------------------

RemovePart::RemovePart(Part* p, size_t partIdx)
{
    m_part = p;
    m_partIdx = partIdx;

    if (m_partIdx == muse::nidx) {
        m_partIdx = muse::indexOf(m_part->score()->parts(), m_part);
    }
}

void RemovePart::undo()
{
    m_part->score()->insertPart(m_part, m_partIdx);
}

void RemovePart::redo()
{
    m_part->score()->removePart(m_part);
}

void RemovePart::cleanup(bool wasDone)
{
    if (wasDone) {
        delete m_part;
        m_part = nullptr;
    }
}

//---------------------------------------------------------
//   SetSoloist
//---------------------------------------------------------

SetSoloist::SetSoloist(Part* p, bool b)
{
    part = p;
    soloist  = b;
}

void SetSoloist::undo()
{
    part->setSoloist(!soloist);
}

void SetSoloist::redo()
{
    part->setSoloist(soloist);
}

//---------------------------------------------------------
//   ChangePart
//---------------------------------------------------------

ChangePart::ChangePart(Part* _part, Instrument* i)
{
    instrument = i;
    part       = _part;
}

void ChangePart::flip()
{
    Instrument* oi = part->instrument(); //tick?
    part->setInstrument(instrument);

    part->updateHarmonyChannels(false);

    Score* score = part->score();
    score->masterScore()->rebuildMidiMapping();
    score->setInstrumentsChanged(true);

    // check if notes need to be updated
    // true if changing into or away from TAB or from one TAB type to another

    score->setLayoutAll();

    instrument = oi;
}

void ChangePart::cleanup(bool)
{
    delete instrument;
    instrument = nullptr;
}

//---------------------------------------------------------
//   ChangeInstrumentLong
//---------------------------------------------------------

ChangeInstrumentLong::ChangeInstrumentLong(const Fraction& _tick, Part* p, const String& t)
    : part(p), tick(_tick), longName(t)
{
}

void ChangeInstrumentLong::flip()
{
    String s = part->longName(tick);
    part->setLongName(longName, tick);
    longName = s;
    part->score()->setLayoutAll();
}

//---------------------------------------------------------
//   ChangeInstrumentShort
//---------------------------------------------------------

ChangeInstrumentShort::ChangeInstrumentShort(const Fraction& _tick, Part* p, const String& t)
    : part(p), tick(_tick), shortName(t)
{
}

void ChangeInstrumentShort::flip()
{
    String s = part->shortName(tick);
    part->setShortName(shortName, tick);
    shortName = s;
    part->score()->setLayoutAll();
}

//---------------------------------------------------------
//   ChangeInstrumentLong
//---------------------------------------------------------

ChangeInstrumentGroupOptions::ChangeInstrumentGroupOptions(const Fraction& _tick, Part* p, bool useCustom, const String& longName,
                                                           const String& shortName)
    : part(p), tick(_tick), useCustom(useCustom), longName(longName), shortName(shortName)
{
}

void ChangeInstrumentGroupOptions::flip()
{
    InstrumentLabel& label = part->instrument(tick)->instrumentLabel();

    bool curUseCustom = label.useCustomGroupName();
    const String& curLong = label.customNameLongGroup();
    const String& curShort = label.customNameShortGroup();

    label.setUseCustomGroupName(useCustom);
    label.setCustomNameLongGroup(longName);
    label.setCustomNameShortGroup(shortName);

    useCustom = curUseCustom;
    longName = curLong;
    shortName = curShort;

    part->score()->setLayoutAll();
}

ChangeInstrumentNumber::ChangeInstrumentNumber(const Fraction& _tick, Part* p, int v)
    : part(p), tick(_tick), number(v)
{
}

void ChangeInstrumentNumber::flip()
{
    int v = part->number(tick);
    part->setNumber(number, tick);
    number = v;
    part->score()->setLayoutAll();
}

//---------------------------------------------------------
//   ChangeDrumset
//---------------------------------------------------------

void ChangeDrumset::flip()
{
    Drumset d = instrument->drumset() ? *instrument->drumset() : Drumset();
    instrument->setDrumset(&drumset);
    drumset = d;

    if (part->staves().size() > 0) {
        part->score()->setLayout(Fraction(0, 1), part->score()->endTick(), part->staves().front()->idx(), part->staves().back()->idx());
    }
}

//---------------------------------------------------------
//   ChangeStringData
//---------------------------------------------------------

void ChangeStringData::flip()
{
    const StringData* stringData =  m_stringTunings ? m_stringTunings->stringData() : m_instrument->stringData();
    int frets = stringData->frets();
    std::vector<instrString> stringList = stringData->stringList();

    if (m_stringTunings) {
        m_stringTunings->setStringData(m_stringData);
    } else {
        m_instrument->setStringData(m_stringData);
    }

    m_stringData.set(StringData(frets, stringList));
}

//---------------------------------------------------------
//   ChangePatch
//---------------------------------------------------------

void ChangePatch::flip()
{
    MidiPatch op;
    op.prog          = channel->program();
    op.bank          = channel->bank();
    op.synti         = channel->synti();

    channel->setProgram(patch.prog);
    channel->setBank(patch.bank);
    channel->setSynti(patch.synti);

    patch            = op;

    //! TODO Needs porting for MU4
//    if (MScore::seq == 0) {
//        LOGW("no seq");
//        return;
//    }
//
//    NPlayEvent event;
//    event.setType(ME_CONTROLLER);
//    event.setChannel(channel->channel());
//
//    int hbank = (channel->bank() >> 7) & 0x7f;
//    int lbank = channel->bank() & 0x7f;
//
//    event.setController(CTRL_HBANK);
//    event.setValue(hbank);
//    MScore::seq->sendEvent(event);
//
//    event.setController(CTRL_LBANK);
//    event.setValue(lbank);
//    MScore::seq->sendEvent(event);
//
//    event.setController(CTRL_PROGRAM);
//    event.setValue(channel->program());
//
    score->setInstrumentsChanged(true);
//
//    MScore::seq->sendEvent(event);
}

//---------------------------------------------------------
//   SetUserBankController
//---------------------------------------------------------

void SetUserBankController::flip()
{
    bool oldVal = channel->userBankController();
    channel->setUserBankController(val);
    val = oldVal;
}

//---------------------------------------------------------
//   findInstrumentChange
//   Helper function to find InstrumentChange element at a given tick
//---------------------------------------------------------

static InstrumentChange* findInstrumentChange(Score* score, const Part* part, const Fraction& tick)
{
    const Segment* segment = score->tick2segment(tick, true, SegmentType::ChordRest);
    if (!segment) {
        return nullptr;
    }

    const TrackRange trackRange = part->trackRange();
    EngravingItem* item = segment->findAnnotation(ElementType::INSTRUMENT_CHANGE, trackRange.startTrack, trackRange.endTrack - 1);
    return item ? toInstrumentChange(item) : nullptr;
}

static std::vector<Score*> excerptScores(const Score* score)
{
    std::vector<Score*> result;
    if (!score->isMaster()) {
        return result;
    }

    for (const Excerpt* excerpt : score->masterScore()->excerpts()) {
        if (Score* excerptScore = excerpt->excerptScore()) {
            result.push_back(excerptScore);
        }
    }

    return result;
}

static std::vector<Part*> partsIn(const Score* score, const std::vector<Part*>& parts)
{
    std::vector<Part*> result;
    for (const Part* part : parts) {
        if (Part* scorePart = score->partById(part->id())) {
            result.push_back(scorePart);
        }
    }

    return result;
}

static std::vector<Staff*> stavesIn(const Score* score, const std::vector<Staff*>& staves)
{
    std::vector<Staff*> result;
    for (const Staff* staff : staves) {
        if (Staff* scoreStaff = staff->findLinkedInScore(score)) {
            result.push_back(scoreStaff);
        }
    }

    return result;
}

void EditPart::replacePartInstrument(Score* score, Part* part, const Instrument& newInstrument,
                                     const StaffType* newStaffType)
{
    if (!score || !part) {
        return;
    }

    score->undo(new ChangePart(part, new Instrument(newInstrument)));

    // Update clefs and staff type for all staves in the part
    for (staff_idx_t staffIdx = 0; staffIdx < part->nstaves(); ++staffIdx) {
        Staff* staff = part->staves().at(staffIdx);
        if (!staff) {
            continue;
        }

        // Get current staff properties
        bool visible = staff->visible();
        ClefTypeList currentClefType = staff->defaultClefType();
        Spatium userDist = staff->userDist();
        bool cutaway = staff->cutaway();
        bool hideSystemBarLine = staff->hideSystemBarLine();
        AutoOnOff mergeMatchingRests = staff->mergeMatchingRests();
        bool reflectTransposition = staff->reflectTranspositionInLinkedTab();

        // Get new clef type from the new instrument
        ClefTypeList newClefType = newInstrument.clefType(staffIdx);

        // Only update staff if clef type changes
        if (currentClefType != newClefType) {
            score->undo(new ChangeStaff(staff, visible, newClefType, userDist, cutaway,
                                        hideSystemBarLine, mergeMatchingRests, reflectTransposition));
        }

        // Apply new staff type if provided
        if (newStaffType) {
            score->undo(new ChangeStaffType(staff, *newStaffType));
        }
    }
}

bool EditPart::replaceInstrument(Score* score, Part* part, const Fraction& tick, const Instrument& newInstrument,
                                 const StaffType* newStaffType)
{
    if (!score || !part) {
        return false;
    }

    if (!score->isMaster()) {
        return doReplaceInstrument(score, part, tick, newInstrument, newStaffType);
    }

    const Interval oldTranspose = part->instrument()->transpose();

    const bool replaced = doReplaceInstrument(score, part, tick, newInstrument, newStaffType);

    for (Score* excerptScore : excerptScores(score)) {
        if (Part* excerptPart = excerptScore->partById(part->id())) {
            doReplaceInstrument(excerptScore, excerptPart, tick, newInstrument, newStaffType);
        }
    }

    // also transposes all linked parts
    if (newInstrument.transpose().reduced() != oldTranspose.reduced()) {
        Transaction& tx = score->transactionManager()->currentOrDummyTransaction();
        Transpose::transpositionChanged(tx, score, part, Part::MAIN_INSTRUMENT_TICK, oldTranspose);
    }

    if (tick == Part::MAIN_INSTRUMENT_TICK) {
        renameExcerptOfPart(score->masterScore(), part);
    }

    return replaced;
}

void EditPart::renameExcerptOfPart(MasterScore* master, const Part* part)
{
    Excerpt* excerptOfPart = nullptr;
    StringList allExcerptLowerNames;

    for (Excerpt* excerpt : master->excerpts()) {
        if (excerpt->initialPartId() == part->id()) {
            excerptOfPart = excerpt;
            continue;
        }
        allExcerptLowerNames.push_back(excerpt->name().toLower());
    }

    if (!excerptOfPart) {
        return;
    }

    const String newName = formatUniqueExcerptName(part->partName(), allExcerptLowerNames);
    excerptOfPart->excerptScore()->undo(new ChangeExcerptTitle(excerptOfPart, newName));
}

bool EditPart::doReplaceInstrument(Score* score, Part* part, const Fraction& tick, const Instrument& newInstrument,
                                   const StaffType* newStaffType)
{
    if (tick == Part::MAIN_INSTRUMENT_TICK) {
        replacePartInstrument(score, part, newInstrument, newStaffType);
        return true;
    }

    InstrumentChange* instrumentChange = findInstrumentChange(score, part, tick);
    if (!instrumentChange) {
        return false;
    }

    instrumentChange->setInit(true);
    instrumentChange->setupInstrument(&newInstrument);
    return true;
}

void EditPart::setPartVisible(Score* score, Part* part, bool visible)
{
    if (!score || !part) {
        return;
    }

    part->undoChangeProperty(Pid::VISIBLE, visible);

    if (visible) {
        removeLocksContainingMMRests(score);
    }
}

void EditPart::removeLocksContainingMMRests(Score* score)
{
    Transaction& tx = score->transactionManager()->currentOrDummyTransaction();
    EditPageLocks::removePageLocksContainingMMRests(tx, score);
    EditSystemLocks::removeSystemLocksContainingMMRests(tx, score);
}

void EditPart::setStaffVisible(Score* score, Staff* staff, bool visible)
{
    if (!score || !staff) {
        return;
    }

    staff->undoChangeProperty(Pid::VISIBLE, visible);
}

void EditPart::setPartSharpFlat(Score* score, Part* part, PreferSharpFlat sharpFlat)
{
    if (!score || !part) {
        return;
    }

    Interval oldTransposition = part->staff(0)->transpose(Fraction(0, 1));

    part->undoChangeProperty(Pid::PREFER_SHARP_FLAT, int(sharpFlat));

    Transaction& tx = score->transactionManager()->currentOrDummyTransaction();
    Transpose::transpositionChanged(tx, score, part, oldTransposition);
}

void EditPart::setInstrumentName(Score* score, Part* part, const Fraction& tick, const String& name)
{
    if (!score || !part) {
        return;
    }

    score->undo(new ChangeInstrumentLong(tick, part, name));
}

void EditPart::setInstrumentAbbreviature(Score* score, Part* part, const Fraction& tick, const String& abbreviature)
{
    if (!score || !part) {
        return;
    }

    score->undo(new ChangeInstrumentShort(tick, part, abbreviature));
}

bool EditPart::setInstrumentGroupNameOptions(Score* score, Part* part, const Fraction& tick, bool useCustom, const String& longName,
                                             const String& shortName)
{
    if (!score || !part) {
        return false;
    }

    const Instrument* instrument = part->instrument(tick);
    if (!instrument) {
        return false;
    }

    const InstrumentLabel& label = instrument->instrumentLabel();
    if (label.useCustomGroupName() == useCustom && label.customNameLongGroup() == longName
        && label.customNameShortGroup() == shortName) {
        return false;
    }

    score->undo(new ChangeInstrumentGroupOptions(tick, part, useCustom, longName, shortName));
    return true;
}

void EditPart::setInstrumentNumber(Score* score, Part* part, const Fraction& tick, int number)
{
    if (!score || !part) {
        return;
    }

    score->undo(new ChangeInstrumentNumber(tick, part, number));
}

void EditPart::setSharedPartEnabled(Score* score, Part* part, bool enabled)
{
    if (!score || !part) {
        return;
    }

    part->undoChangeProperty(Pid::SHARED_PART_ENABLED, enabled);
}

void EditPart::setStaffType(Score* score, Staff* staff, StaffTypes typeId)
{
    if (!score || !staff) {
        return;
    }

    const StaffType* staffType = StaffType::preset(typeId);
    if (!staffType) {
        return;
    }

    score->undo(new ChangeStaffType(staff, *staffType));
}

void EditPart::setStaffConfig(Score* score, Staff* staff, const StaffConfig& config, const Fraction& tick)
{
    if (!score || !staff || !staff->staffType(tick)) {
        return;
    }

    const bool becomesVisible = config.visible && !staff->visible();

    score->undo(new ChangeStaff(staff, config.visible, config.clefTypeList, config.userDistance, config.cutaway,
                                config.hideSystemBarline, config.mergeMatchingRests, config.reflectTranspositionInLinkedTab));

    score->undo(new ChangeStaffType(staff, config.staffType, tick));

    if (becomesVisible) {
        removeLocksContainingMMRests(score);
    }
}

void EditPart::removeParts(Score* score, const std::vector<Part*>& parts)
{
    if (!score || parts.empty()) {
        return;
    }

    doRemoveParts(score, parts);

    if (score->isMaster()) {
        deleteExcerptsOfParts(score->masterScore(), parts);
    }

    for (Score* excerptScore : excerptScores(score)) {
        const std::vector<Part*> excerptParts = partsIn(excerptScore, parts);
        if (!excerptParts.empty()) {
            doRemoveParts(excerptScore, excerptParts);
        }
    }
}

void EditPart::doRemoveParts(Score* score, const std::vector<Part*>& parts)
{
    for (Part* part : parts) {
        score->cmdRemovePart(part);
    }

    score->setBracketsAndBarlines();
}

void EditPart::deleteExcerptsOfParts(MasterScore* master, const std::vector<Part*>& parts)
{
    const std::vector<Excerpt*> excerpts = master->excerpts();

    for (Excerpt* excerpt : excerpts) {
        const muse::ID& initialPartId = excerpt->initialPartId();
        if (!initialPartId.isValid()) {
            continue;
        }

        const bool isPartRemoved = std::find_if(parts.cbegin(), parts.cend(), [&initialPartId](const Part* part) {
            return part->id() == initialPartId;
        }) != parts.cend();

        if (isPartRemoved) {
            master->deleteExcerpt(excerpt);
        }
    }
}

void EditPart::removeStaves(Score* score, const std::vector<Staff*>& staves)
{
    if (!score || staves.empty()) {
        return;
    }

    doRemoveStaves(score, staves);

    for (Score* excerptScore : excerptScores(score)) {
        const std::vector<Staff*> excerptStaves = stavesIn(excerptScore, staves);
        if (!excerptStaves.empty()) {
            doRemoveStaves(excerptScore, excerptStaves);
        }
    }
}

void EditPart::doRemoveStaves(Score* score, const std::vector<Staff*>& staves)
{
    for (Staff* staff : staves) {
        score->cmdRemoveStaff(staff->idx());
    }

    score->setBracketsAndBarlines();
}

void EditPart::moveParts(Score* score, const std::vector<Part*>& sourceParts, Part* destinationPart, bool insertAfter)
{
    if (!score || sourceParts.empty() || !destinationPart) {
        return;
    }

    // Build new part order: remove source parts, then insert at destination
    std::vector<Part*> allParts(score->parts().begin(), score->parts().end());

    // Remove source parts from the list
    for (Part* srcPart : sourceParts) {
        allParts.erase(std::remove(allParts.begin(), allParts.end(), srcPart), allParts.end());
    }

    // Find destination index
    auto dstIt = std::find(allParts.begin(), allParts.end(), destinationPart);
    if (dstIt == allParts.end()) {
        return;
    }

    size_t dstIndex = std::distance(allParts.begin(), dstIt);
    if (insertAfter) {
        dstIndex++;
    }

    // Insert source parts at destination
    for (size_t i = 0; i < sourceParts.size(); ++i) {
        allParts.insert(allParts.begin() + dstIndex + i, sourceParts[i]);
    }

    // Build staff index mapping from the new part order
    std::vector<staff_idx_t> staffMapping;
    staffMapping.reserve(score->nstaves());
    for (Part* part : allParts) {
        for (Staff* staff : part->staves()) {
            staffMapping.push_back(staff->idx());
        }
    }

    score->undo(new SortStaves(score, staffMapping));
    score->setBracketsAndBarlines();
}

void EditPart::moveStaves(Score* score, const std::vector<Staff*>& sourceStaves, Staff* destinationStaff, bool insertAfter)
{
    if (!score || sourceStaves.empty() || !destinationStaff) {
        return;
    }

    // Build new staff order: remove source staves, then insert at destination
    std::vector<Staff*> allStaves(score->staves().begin(), score->staves().end());

    // Remove source staves from the list
    for (Staff* srcStaff : sourceStaves) {
        allStaves.erase(std::remove(allStaves.begin(), allStaves.end(), srcStaff), allStaves.end());
    }

    // Find destination index
    auto dstIt = std::find(allStaves.begin(), allStaves.end(), destinationStaff);
    if (dstIt == allStaves.end()) {
        return;
    }

    size_t dstIndex = std::distance(allStaves.begin(), dstIt);
    if (insertAfter) {
        dstIndex++;
    }

    // Insert source staves at destination
    for (size_t i = 0; i < sourceStaves.size(); ++i) {
        allStaves.insert(allStaves.begin() + dstIndex + i, sourceStaves[i]);
    }

    // Build index mapping
    std::vector<staff_idx_t> sortedIndexes;
    sortedIndexes.reserve(allStaves.size());
    for (const Staff* staff : allStaves) {
        sortedIndexes.push_back(staff->idx());
    }

    score->undo(new SortStaves(score, sortedIndexes));
    score->setBracketsAndBarlines();
}

void EditPart::addSystemObjects(Score* score, const std::vector<Staff*>& staves)
{
    if (!score || staves.empty()) {
        return;
    }

    std::vector<EngravingItem*> topSystemObjects = collectSystemObjects(score);

    for (Staff* staff : staves) {
        if (staff->isSystemObjectStaff()) {
            continue;
        }

        score->undo(new AddSystemObjectStaff(staff));

        const staff_idx_t staffIdx = staff->idx();
        for (EngravingItem* obj : topSystemObjects) {
            if (obj->isTimeSig()) {
                obj->triggerLayout();
                continue;
            }
            EngravingItem* copy = obj->linkedClone();
            copy->setStaffIdx(staffIdx);
            score->undoAddElement(copy, false /*addToLinkedStaves*/);
        }
    }
}

void EditPart::removeSystemObjects(Score* score, const std::vector<Staff*>& staves)
{
    if (!score || staves.empty()) {
        return;
    }

    std::vector<EngravingItem*> systemObjects = collectSystemObjects(score, staves);

    for (Staff* staff : staves) {
        if (!staff->isSystemObjectStaff()) {
            continue;
        }

        if (staff->hasSystemObjectsBelowBottomStaff()) {
            score->undoChangeStyleVal(Sid::systemObjectsBelowBottomStaff, false);
        }
        score->undo(new RemoveSystemObjectStaff(staff));
    }

    for (EngravingItem* obj : systemObjects) {
        if (obj->isTimeSig()) {
            obj->triggerLayout();
            continue;
        }
        obj->undoUnlink();
        score->undoRemoveElement(obj, false /*removeLinked*/);
    }
}

void EditPart::moveSystemObjects(Score* score, Staff* sourceStaff, Staff* destinationStaff)
{
    if (!score || !sourceStaff || !destinationStaff) {
        return;
    }

    if (!sourceStaff->isSystemObjectStaff()) {
        return;
    }

    const std::vector<EngravingItem*> systemObjects = collectSystemObjects(score, { sourceStaff, destinationStaff });
    const staff_idx_t dstStaffIdx = destinationStaff->idx();

    score->undo(new RemoveSystemObjectStaff(sourceStaff));
    if (!destinationStaff->isSystemObjectStaff() && dstStaffIdx != 0) {
        score->undo(new AddSystemObjectStaff(destinationStaff));
    } else {
        score->undoChangeStyleVal(Sid::systemObjectsBelowBottomStaff, false);
    }

    AutoOnOff showMeasNumOnSrcStaff = sourceStaff->getProperty(Pid::SHOW_MEASURE_NUMBERS).value<AutoOnOff>();
    if (showMeasNumOnSrcStaff != AutoOnOff::AUTO) {
        destinationStaff->undoChangeProperty(Pid::SHOW_MEASURE_NUMBERS, showMeasNumOnSrcStaff);
        sourceStaff->undoResetProperty(Pid::SHOW_MEASURE_NUMBERS);
    }

    // Remove items from destination first
    for (EngravingItem* item : systemObjects) {
        if (item->isTimeSig()) {
            item->triggerLayout();
            continue;
        }

        if (item->staff() == sourceStaff) {
            continue;
        }
        item->undoUnlink();
        score->undoRemoveElement(item, false /*removeLinked*/);
    }

    // Move items from source to destination
    for (EngravingItem* item : systemObjects) {
        if (item->isTimeSig()) {
            continue;
        }

        if (item->staff() == sourceStaff) {
            item->undoChangeProperty(Pid::TRACK, staff2track(dstStaffIdx, item->voice()));
        }
    }
}

bool EditPart::reorderPartsChangesScore(const Score* score, const std::vector<Part*>& parts,
                                        const std::vector<SystemObjectsMove>& moves)
{
    if (!score || score->staves().empty()) {
        return false;
    }

    const std::vector<Part*>& scoreParts = score->parts();
    if (parts.size() == scoreParts.size() && !std::equal(parts.begin(), parts.end(), scoreParts.begin())) {
        return true;
    }

    if (moves.empty()) {
        return false;
    }

    const Staff* bottomStaff = score->staves().back();
    const bool currentBelowBottomStaff = score->style().styleB(Sid::systemObjectsBelowBottomStaff);
    bool belowBottomStaff = currentBelowBottomStaff;

    for (const SystemObjectsMove& move : moves) {
        if (move.staff != (move.belowBottomStaff ? bottomStaff : move.destination)) {
            return true;
        }

        if (move.staff == bottomStaff) {
            belowBottomStaff = move.belowBottomStaff;
        }
    }

    return (belowBottomStaff && bottomStaff->isSystemObjectStaff()) != currentBelowBottomStaff;
}

void EditPart::reorderParts(Score* score, const std::vector<Part*>& parts, const std::vector<SystemObjectsMove>& moves)
{
    if (!score || score->staves().empty()) {
        return;
    }

    doReorderParts(score, parts, moves);

    Staff* scoreBottomStaff = score->staves().back();

    // part scores keep their own instrument order, only the markings move
    for (Score* excerptScore : excerptScores(score)) {
        if (excerptScore->staves().empty()) {
            continue;
        }

        const Staff* bottomStaff = excerptScore->staves().back();

        std::vector<SystemObjectsMove> excerptMoves;
        for (const SystemObjectsMove& move : moves) {
            Staff* targetStaff = move.destination;
            if (!targetStaff && move.belowBottomStaff) {
                targetStaff = scoreBottomStaff;
            }

            if (!move.staff || !targetStaff) {
                continue;
            }

            Staff* staff = move.staff->findLinkedInScore(excerptScore);
            Staff* destination = targetStaff->findLinkedInScore(excerptScore);
            if (!staff || !destination) {
                continue;
            }

            excerptMoves.push_back({ staff, destination, move.belowBottomStaff && destination == bottomStaff });
        }

        doReorderParts(excerptScore, excerptScore->parts(), excerptMoves);
    }
}

void EditPart::doReorderParts(Score* score, const std::vector<Part*>& parts, const std::vector<SystemObjectsMove>& moves)
{
    // Sorting keeps the markings at their staff positions, so their indexes are taken first
    std::vector<std::pair<staff_idx_t, Staff*> > pendingMoves;
    pendingMoves.reserve(moves.size());

    for (const SystemObjectsMove& move : moves) {
        if (move.staff && move.staff->isSystemObjectStaff() && (move.destination || move.belowBottomStaff)) {
            pendingMoves.emplace_back(move.staff->idx(), move.belowBottomStaff ? nullptr : move.destination);
        }
    }

    const Staff* staffWithMarkingsBelow = score->style().styleB(Sid::systemObjectsBelowBottomStaff)
                                          ? score->staves().back() : nullptr;

    const std::vector<Part*>& scoreParts = score->parts();
    if (parts.size() == scoreParts.size() && !std::equal(parts.begin(), parts.end(), scoreParts.begin())) {
        std::vector<staff_idx_t> staffMapping;
        staffMapping.reserve(score->nstaves());
        for (const Part* part : parts) {
            for (const Staff* staff : part->staves()) {
                staffMapping.push_back(staff->idx());
            }
        }

        score->undo(new SortStaves(score, staffMapping));
        score->setBracketsAndBarlines();
    }

    const bool belowBottomStaff = moveSystemObjects(score, pendingMoves, staffWithMarkingsBelow == score->staves().back());
    if (belowBottomStaff != score->style().styleB(Sid::systemObjectsBelowBottomStaff)) {
        score->undoChangeStyleVal(Sid::systemObjectsBelowBottomStaff, belowBottomStaff);
    }
}

bool EditPart::reorderStavesChangesScore(const std::vector<Staff*>& staves)
{
    if (staves.empty() || !staves.front()) {
        return false;
    }

    const Part* part = staves.front()->part();
    const std::vector<Staff*>& partStaves = part->staves();
    if (staves.size() != partStaves.size()) {
        return false;
    }

    for (const Staff* staff : staves) {
        IF_ASSERT_FAILED_X(staff && staff->part() == part, "All staves must have the same part!") {
            return false;
        }
    }

    return !std::equal(staves.begin(), staves.end(), partStaves.begin());
}

void EditPart::reorderStaves(Score* score, const std::vector<Staff*>& staves)
{
    if (!score) {
        return;
    }

    std::vector<staff_idx_t> staffMapping;
    staffMapping.reserve(score->nstaves());
    for (const Staff* staff : score->staves()) {
        staffMapping.push_back(staff->idx());
    }

    // The part's staff slots get its staves in the new order
    const staff_idx_t firstStaffIdx = staves.front()->part()->staves().front()->idx();
    for (size_t i = 0; i < staves.size(); ++i) {
        staffMapping[firstStaffIdx + i] = staves[i]->idx();
    }

    score->undo(new SortStaves(score, staffMapping));
    score->setBracketsAndBarlines();
}

bool EditPart::moveSystemObjects(Score* score, std::vector<std::pair<staff_idx_t, Staff*> > moves, bool belowBottomStaff)
{
    Staff* bottomStaff = score->staves().back();

    auto hasPendingMoveFrom = [&moves](staff_idx_t staffIdx) {
        return std::any_of(moves.begin(), moves.end(), [staffIdx](const std::pair<staff_idx_t, Staff*>& move) {
            return move.first == staffIdx;
        });
    };

    // Waits for the destination's own markings to move away, otherwise replaces them
    bool moved = true;
    while (!moves.empty() && moved) {
        moved = false;

        for (auto it = moves.begin(); it != moves.end();) {
            Staff* from = score->staff(it->first);
            Staff* to = it->second ? it->second : bottomStaff;
            if (!from || (from != to && hasPendingMoveFrom(to->idx()))) {
                ++it;
                continue;
            }

            if (to == bottomStaff) {
                belowBottomStaff = !it->second;
            }
            if (from != to) {
                moveSystemObjects(score, from, to);
            }

            it = moves.erase(it);
            moved = true;
        }
    }

    return belowBottomStaff && bottomStaff->isSystemObjectStaff();
}

void EditPart::appendStaff(Score* score, Staff* staff, Part* destinationPart, bool createRests)
{
    if (!score || !staff || !destinationPart) {
        return;
    }

    doAppendStaff(score, staff, destinationPart, createRests);

    for (Score* excerptScore : excerptScores(score)) {
        if (Part* excerptPart = excerptScore->partById(destinationPart->id())) {
            Staff* excerptStaff = staff->clone();
            doAppendStaff(excerptScore, excerptStaff, excerptPart, false);
            excerptScore->undo(new Link(excerptStaff, staff));
            Excerpt::cloneStaff2(staff, excerptStaff, Fraction(0, 1), excerptScore->endTick());
        }
    }
}

void EditPart::doAppendStaff(Score* score, Staff* staff, Part* destinationPart, bool createRests)
{
    if (score->isMaster()) {
        // assigned when added to the score, then shared with the part scores
        staff->setId(INVALID_ID);
    }

    staff_idx_t staffLocalIndex = destinationPart->nstaves();
    Staff* lastStaff = staffLocalIndex > 0 ? destinationPart->staff(staffLocalIndex - 1) : nullptr;
    KeyList keyList = lastStaff ? *lastStaff->keyList() : score->keyList();

    staff->setScore(score);
    staff->setPart(destinationPart);

    score->undoInsertStaff(staff, staffLocalIndex, createRests);

    staff_idx_t staffGlobalIndex = staff->idx();
    score->adjustKeySigs(staffGlobalIndex, staffGlobalIndex + 1, keyList);

    score->updateBracesAndBarlines(destinationPart, staffLocalIndex);
    score->setBracketsAndBarlines();

    destinationPart->instrument()->setClefType(staffLocalIndex, staff->defaultClefType());
}

Staff* EditPart::appendStaff(Score* score, Part* destinationPart)
{
    if (!score || !destinationPart) {
        return nullptr;
    }

    Staff* staff = Factory::createStaff(destinationPart);
    appendStaff(score, staff, destinationPart);
    return staff;
}

void EditPart::appendLinkedStaff(Score* score, Staff* staff, Staff* sourceStaff, Part* destinationPart)
{
    if (!score || !staff || !sourceStaff || !destinationPart) {
        return;
    }

    doAppendLinkedStaff(score, staff, sourceStaff, destinationPart);

    for (Score* excerptScore : excerptScores(score)) {
        Staff* excerptSourceStaff = sourceStaff->findLinkedInScore(excerptScore);
        Part* excerptPart = excerptScore->partById(destinationPart->id());
        if (excerptSourceStaff && excerptPart) {
            doAppendLinkedStaff(excerptScore, staff->clone(), excerptSourceStaff, excerptPart);
        }
    }
}

void EditPart::doAppendLinkedStaff(Score* score, Staff* staff, Staff* sourceStaff, Part* destinationPart)
{
    doAppendStaff(score, staff, destinationPart, false);

    staff->setLinks(nullptr);
    Excerpt::cloneStaff(sourceStaff, staff);
}

Staff* EditPart::appendLinkedStaff(Score* score, Staff* sourceStaff, Part* destinationPart)
{
    if (!score || !sourceStaff || !destinationPart) {
        return nullptr;
    }

    Staff* staff = Factory::createStaff(destinationPart);
    appendLinkedStaff(score, staff, sourceStaff, destinationPart);
    return staff;
}

void EditPart::appendStaves(Score* score, Part* part, const InstrumentTemplate& templ, const KeyList& keyList)
{
    if (!score || !part) {
        return;
    }

    const StaffType* staffType = templ.staffTypePreset;
    if (!staffType) {
        staffType = StaffType::preset(StaffTypes::STANDARD);
    }

    for (staff_idx_t staffIndex = 0; staffIndex < templ.staffCount; ++staffIndex) {
        Staff* staff = Factory::createStaff(part);
        score->undoInsertStaff(staff, staffIndex);
        staff->init(&templ, staffType, staffIndex);
    }

    if (!part->nstaves()) {
        return;
    }

    staff_idx_t firstStaffIndex = part->staff(0)->idx();
    staff_idx_t endStaffIndex = firstStaffIndex + part->nstaves();
    score->adjustKeySigs(firstStaffIndex, endStaffIndex, keyList);
}

bool EditPart::setVoiceVisible(Score* score, Staff* staff, int voiceIndex, bool visible)
{
    if (!score || !staff) {
        return false;
    }

    if (!score->excerpt()) {
        return false;
    }

    if (!visible && !staff->canDisableVoice()) {
        return false;
    }

    score->excerpt()->setVoiceVisible(staff, voiceIndex, visible);
    return true;
}

void EditPart::insertPart(Score* score, const InstrumentTemplate* templ, size_t index)
{
    if (!score || !templ) {
        return;
    }

    insertPart(score, *templ, index, score->keyList());
    score->masterScore()->rebuildMidiMapping();
}

Part* EditPart::insertPart(Score* score, const InstrumentTemplate& templ, size_t index, const KeyList& keyList)
{
    if (!score) {
        return nullptr;
    }

    Instrument instrument = Instrument::fromTemplate(&templ);
    const String& longName = instrument.longName();
    const String& shortName = instrument.shortName();

    const Trait trait = instrument.trait();
    const String& transposition = trait.type == TraitType::Transposition && !trait.isHiddenOnScore ? trait.name : String();

    Part* part = new Part(score);
    part->setInstrument(instrument);

    part->setLongName(muse::mtrc("notation", longName));
    part->setShortName(muse::mtrc("notation", shortName));
    part->setTransposition(muse::mtrc("notation", transposition));

    doInsertPart(score, part, index);
    appendStaves(score, part, templ, keyList);

    return part;
}

void EditPart::insertPart(Score* score, Part* part, size_t index)
{
    if (!score || !part) {
        return;
    }

    removeLocksContainingMMRests(score);
    insertPartCopy(score, part, index);
}

void EditPart::doInsertPart(Score* score, Part* part, size_t index)
{
    if (Excerpt* excerpt = score->excerpt()) {
        score->undo(new AddPartToExcerpt(excerpt, part, index));
    } else {
        score->undoInsertPart(part, index);
    }
}

void EditPart::insertPartCopy(Score* score, Part* part, size_t index)
{
    std::vector<Staff*> stavesCopy(part->staves());
    part->clearStaves();

    InstrumentList instrumentsCopy = part->instruments();
    part->setInstruments({});

    doInsertPart(score, part, index);

    for (auto it = instrumentsCopy.cbegin(); it != instrumentsCopy.cend(); ++it) {
        part->setInstrument(new Instrument(*it->second), it->first);
    }

    Fraction startTick = score->firstMeasure()->tick();
    Fraction endTick = score->lastMeasure()->endTick();

    for (size_t staffIndex = 0; staffIndex < stavesCopy.size(); ++staffIndex) {
        Staff* staff = stavesCopy[staffIndex];

        Staff* staffCopy = Factory::createStaff(part);
        staffCopy->setId(staff->id());
        staffCopy->setScore(score);
        staffCopy->setPart(part);
        staffCopy->init(staff);

        score->undoInsertStaff(staffCopy, staffIndex, /* createRests = */ false);
        score->undo(new Link(staffCopy, staff));

        Excerpt::cloneStaff2(staff, staffCopy, startTick, endTick);
    }

    score->remapBracketsAndBarlines();
}

EditPart::SetPartsResult EditPart::setParts(Score* score, const std::vector<PartEntry>& entries, const ScoreOrder& order)
{
    if (!score) {
        return {};
    }

    const KeyList keyList = score->keyList();

    SetPartsResult result;

    setScoreOrder(score, order);
    result.removedParts = removeMissingParts(score, entries);
    result.addedParts = insertNewParts(score, entries, keyList);
    updateSoloist(score, entries);
    sortParts(score, entries);
    score->setBracketsAndBarlines();

    return result;
}

std::vector<Part*> EditPart::removeMissingParts(Score* score, const std::vector<PartEntry>& entries)
{
    std::unordered_set<Part*> keptParts;
    for (const PartEntry& entry : entries) {
        if (entry.part) {
            keptParts.insert(entry.part);
        }
    }

    std::vector<Part*> partsToRemove;
    for (Part* part : score->parts()) {
        if (!muse::contains(keptParts, part)) {
            partsToRemove.push_back(part);
        }
    }

    removeParts(score, partsToRemove);

    return partsToRemove;
}

std::vector<Part*> EditPart::insertNewParts(Score* score, const std::vector<PartEntry>& entries, const KeyList& keyList)
{
    std::vector<Part*> newParts;
    if (!score) {
        return newParts;
    }

    const std::vector<int> numbers = resolveInstrumentNumbers(entries);

    for (size_t i = 0; i < entries.size(); ++i) {
        const PartEntry& entry = entries[i];
        const int number = numbers.at(i);

        if (!entry.instrumentTemplate) {
            if (entry.part && number >= 0 && entry.part->number() != number) {
                setInstrumentNumber(score, entry.part, Part::MAIN_INSTRUMENT_TICK, number);
            }
            continue;
        }

        Part* part = insertPart(score, *entry.instrumentTemplate, i, keyList);
        part->setSoloist(entry.isSoloist);
        part->setNumber(std::max(number, 0));
        newParts.push_back(part);
    }

    return newParts;
}

std::vector<int> EditPart::resolveInstrumentNumbers(const std::vector<PartEntry>& entries)
{
    // Kinds with an instrument being added are numbered in their (new) order, unless there's only one. Shared staves
    // and instruments numbered in their name (e.g. "Violins 1") aren't numbered, though the latter still count.
    // Returns the number for each instrument, or -1 to leave it as it is
    struct Counter {
        int count = 0;
        size_t firstIndex = 0; // in entries / numbers: the first instrument of this kind
    };

    std::map<String, Counter> counters;
    for (const PartEntry& entry : entries) {
        if (entry.instrumentTemplate) {
            counters.try_emplace(entry.instrumentTemplate->id);
        }
    }

    std::vector<int> numbers;
    numbers.reserve(entries.size());

    for (const PartEntry& entry : entries) {
        Counter* counter = nullptr; // null: not renumbered
        bool isNumberedInName = false;

        if (entry.instrumentTemplate) {
            counter = &counters.at(entry.instrumentTemplate->id);
        } else if (const Part* part = entry.part; part && !part->isSharedPart()) {
            const Instrument* instrument = part->instrument();
            auto it = counters.find(instrument->id());

            if (it != counters.end()) {
                const String& name = instrument->longName();

                counter = &it->second;
                isNumberedInName = !name.empty() && name.back().isDigit();
            }
        }

        if (!counter) {
            numbers.push_back(-1);
            continue;
        }

        if (counter->count == 0) {
            counter->firstIndex = numbers.size();
        }
        ++counter->count;

        numbers.push_back(isNumberedInName ? 0 : counter->count);
    }

    // An instrument that's the only one of its kind isn't numbered
    for (const auto& [instrumentId, counter] : counters) {
        if (counter.count == 1) {
            numbers[counter.firstIndex] = 0;
        }
    }

    return numbers;
}

void EditPart::updateSoloist(Score* score, const std::vector<PartEntry>& entries)
{
    for (const PartEntry& entry : entries) {
        Part* part = entry.part;
        if (part && !part->isSharedPart() && entry.isSoloist != part->soloist()) {
            score->undo(new SetSoloist(part, entry.isSoloist));
        }
    }
}

void EditPart::sortParts(Score* score, const std::vector<PartEntry>& entries)
{
    const std::vector<Part*>& scoreParts = score->parts();

    std::vector<Part*> orderedParts;
    orderedParts.reserve(entries.size());

    for (size_t i = 0; i < entries.size(); ++i) {
        const PartEntry& entry = entries[i];
        Part* part = entry.instrumentTemplate ? muse::value(scoreParts, i) : entry.part;
        if (part) {
            orderedParts.push_back(part);
        }
    }

    sortStaves(score, orderedParts);
}

void EditPart::sortStaves(Score* score, const std::vector<Part*>& orderedParts)
{
    std::vector<staff_idx_t> staffMapping;

    for (const Part* part : orderedParts) {
        for (Staff* staff : part->staves()) {
            staffMapping.push_back(muse::indexOf(score->staves(), staff));
        }
    }

    score->undo(new SortStaves(score, staffMapping));
}

void EditPart::replacePart(Score* score, Part* oldPart, const InstrumentTemplate* templ)
{
    if (!score || !oldPart || !templ) {
        return;
    }

    size_t partIndex = muse::indexOf(score->parts(), oldPart);
    score->cmdRemovePart(oldPart);
    insertPart(score, templ, partIndex);
}

void EditPart::replacePart(Score* score, Part* oldPart, Part* newPart)
{
    if (!score || !oldPart || !newPart) {
        return;
    }

    size_t partIndex = muse::indexOf(score->parts(), oldPart);
    score->cmdRemovePart(oldPart);
    insertPartCopy(score, newPart, partIndex);
}

void EditPart::replaceDrumset(Score* score, Part* part, const Fraction& tick, const Drumset& newDrumset, bool undoable)
{
    if (!score || !part) {
        return;
    }

    doReplaceDrumset(score, part, tick, newDrumset, undoable);

    for (Score* excerptScore : excerptScores(score)) {
        if (Part* excerptPart = excerptScore->partById(part->id())) {
            doReplaceDrumset(excerptScore, excerptPart, tick, newDrumset, undoable);
        }
    }
}

void EditPart::doReplaceDrumset(Score* score, Part* part, const Fraction& tick, const Drumset& newDrumset, bool undoable)
{
    Instrument* instrument = part->instrument(tick);
    if (!instrument) {
        return;
    }

    if (undoable) {
        score->undo(new ChangeDrumset(instrument, newDrumset, part));
    } else {
        instrument->setDrumset(&newDrumset);
    }
}

void EditPart::setScoreOrder(Score* score, const ScoreOrder& order)
{
    if (!score) {
        return;
    }

    score->undo(new ChangeScoreOrder(score, order));
    ScoreOrder mutableOrder = order;
    mutableOrder.setBracketsAndBarlines(score);
}
