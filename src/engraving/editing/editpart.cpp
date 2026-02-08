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

#include "../dom/excerpt.h"
#include "../dom/instrchange.h"
#include "../dom/masterscore.h"
#include "../dom/part.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/staff.h"
#include "../dom/stafftype.h"
#include "../dom/stringtunings.h"

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

ChangeInstrumentLong::ChangeInstrumentLong(const Fraction& _tick, Part* p, const StaffNameList& t)
    : part(p), tick(_tick), text(t)
{
}

void ChangeInstrumentLong::flip(EditData*)
{
    StaffNameList s = part->longNames(tick);
    part->setLongNames(text, tick);
    text = s;
    part->score()->setLayoutAll();
}

//---------------------------------------------------------
//   ChangeInstrumentShort
//---------------------------------------------------------

ChangeInstrumentShort::ChangeInstrumentShort(const Fraction& _tick, Part* p, const StaffNameList& t)
    : part(p), tick(_tick), text(t)
{
}

void ChangeInstrumentShort::flip(EditData*)
{
    StaffNameList s = part->shortNames(tick);
    part->setShortNames(text, tick);
    text = s;
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
