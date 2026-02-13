/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "editstaff.h"
#include "transpose.h"

#include "../dom/excerpt.h"
#include "../dom/instrchange.h"
#include "../dom/masterscore.h"
#include "../dom/mscore.h"
#include "../dom/part.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/staff.h"
#include "../dom/stafftype.h"
#include "../dom/stringtunings.h"
#include "../dom/utils.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   InsertPart
//---------------------------------------------------------

InsertPart::InsertPart(Part* p, size_t targetPartIdx)
{
    m_part = p;
    m_targetPartIdx = targetPartIdx;
}

void InsertPart::undo(EditData*)
{
    m_part->score()->removePart(m_part);
}

void InsertPart::redo(EditData*)
{
    m_part->score()->insertPart(m_part, m_targetPartIdx);
}

void InsertPart::cleanup(bool undo)
{
    if (!undo) {
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

void RemovePart::undo(EditData*)
{
    m_part->score()->insertPart(m_part, m_partIdx);
}

void RemovePart::redo(EditData*)
{
    m_part->score()->removePart(m_part);
}

void RemovePart::cleanup(bool undo)
{
    if (undo) {
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

void SetSoloist::undo(EditData*)
{
    part->setSoloist(!soloist);
}

void SetSoloist::redo(EditData*)
{
    part->setSoloist(soloist);
}

//---------------------------------------------------------
//   ChangePart
//---------------------------------------------------------

ChangePart::ChangePart(Part* _part, Instrument* i, const String& s)
{
    instrument = i;
    part       = _part;
    partName   = s;
}

void ChangePart::flip(EditData*)
{
    Instrument* oi = part->instrument(); //tick?
    String s      = part->partName();
    part->setInstrument(instrument);
    part->setPartName(partName);

    part->updateHarmonyChannels(false);

    Score* score = part->score();
    score->masterScore()->rebuildMidiMapping();
    score->setInstrumentsChanged(true);
    score->setPlaylistDirty();

    // check if notes need to be updated
    // true if changing into or away from TAB or from one TAB type to another

    score->setLayoutAll();

    partName   = s;
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

ChangeInstrumentLong::ChangeInstrumentLong(const Fraction& _tick, Part* p, const StaffName& t)
    : part(p), tick(_tick), longName(t)
{
}

void ChangeInstrumentLong::flip(EditData*)
{
    StaffName s = part->longName(tick);
    part->setLongName(longName, tick);
    longName = s;
    part->score()->setLayoutAll();
}

//---------------------------------------------------------
//   ChangeInstrumentShort
//---------------------------------------------------------

ChangeInstrumentShort::ChangeInstrumentShort(const Fraction& _tick, Part* p, const StaffName& t)
    : part(p), tick(_tick), shortName(t)
{
}

void ChangeInstrumentShort::flip(EditData*)
{
    StaffName s = part->shortName(tick);
    part->setShortName(shortName, tick);
    shortName = s;
    part->score()->setLayoutAll();
}

//---------------------------------------------------------
//   ChangeDrumset
//---------------------------------------------------------

void ChangeDrumset::flip(EditData*)
{
    Drumset d = *instrument->drumset();
    instrument->setDrumset(&drumset);
    drumset = d;

    if (part->staves().size() > 0) {
        part->score()->setLayout(Fraction(0, 1), part->score()->endTick(), part->staves().front()->idx(), part->staves().back()->idx());
    }
}

//---------------------------------------------------------
//   ChangeStringData
//---------------------------------------------------------

void ChangeStringData::flip(EditData*)
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

void ChangePatch::flip(EditData*)
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

void SetUserBankController::flip(EditData*)
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

    EngravingItem* item = segment->findAnnotation(ElementType::INSTRUMENT_CHANGE, part->startTrack(), part->endTrack() - 1);
    return item ? toInstrumentChange(item) : nullptr;
}

void EditPart::replacePartInstrument(Score* score, Part* part, const Instrument& newInstrument,
                                     const StaffType* newStaffType, const String& partName)
{
    if (!score || !part) {
        return;
    }

    // Change the part's instrument and name
    String newPartName = partName.isEmpty() ? newInstrument.trackName() : partName;
    score->undo(new ChangePart(part, new Instrument(newInstrument), newPartName));

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

bool EditPart::replaceInstrumentAtTick(Score* score, Part* part, const Fraction& tick,
                                       const Instrument& newInstrument)
{
    if (!score || !part) {
        return false;
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
}

void EditPart::setStaffVisible(Score* score, Staff* staff, bool visible)
{
    if (!score || !staff) {
        return;
    }

    score->undo(new ChangeStaff(staff, visible, staff->defaultClefType(), staff->userDist(), staff->cutaway(),
                                staff->hideSystemBarLine(), staff->mergeMatchingRests(),
                                staff->reflectTranspositionInLinkedTab()));
}

void EditPart::setPartSharpFlat(Score* score, Part* part, PreferSharpFlat sharpFlat)
{
    if (!score || !part) {
        return;
    }

    Interval oldTransposition = part->staff(0)->transpose(Fraction(0, 1));

    part->undoChangeProperty(Pid::PREFER_SHARP_FLAT, int(sharpFlat));
    Transpose::transpositionChanged(score, part, oldTransposition);
}

void EditPart::setInstrumentName(Score* score, Part* part, const Fraction& tick, const String& name)
{
    if (!score || !part) {
        return;
    }

    score->undo(new ChangeInstrumentLong(tick, part, StaffName(name)));
}

void EditPart::setInstrumentAbbreviature(Score* score, Part* part, const Fraction& tick, const String& abbreviature)
{
    if (!score || !part) {
        return;
    }

    score->undo(new ChangeInstrumentShort(tick, part, StaffName(abbreviature)));
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

void EditPart::removeParts(Score* score, const std::vector<Part*>& parts)
{
    if (!score || parts.empty()) {
        return;
    }

    for (Part* part : parts) {
        score->cmdRemovePart(part);
    }

    score->setBracketsAndBarlines();
}

void EditPart::removeStaves(Score* score, const std::vector<Staff*>& staves)
{
    if (!score || staves.empty()) {
        return;
    }

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
        if (staff->isSystemObjectStaff()) {
            score->undo(new RemoveSystemObjectStaff(staff));
            if (staff->hasSystemObjectsBelowBottomStaff()) {
                score->undoChangeStyleVal(Sid::systemObjectsBelowBottomStaff, false);
            }
        }
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

void EditPart::setStaffConfig(Score* score, Staff* staff, bool visible, double userDistance,
                              bool cutaway, bool hideSystemBarLine, int mergeMatchingRests,
                              bool reflectTransposition, StaffTypes staffTypeId)
{
    if (!score || !staff) {
        return;
    }

    score->undo(new ChangeStaff(staff, visible, staff->defaultClefType(), Spatium(userDistance), cutaway,
                                hideSystemBarLine, AutoOnOff(mergeMatchingRests), reflectTransposition));

    setStaffType(score, staff, staffTypeId);
}
