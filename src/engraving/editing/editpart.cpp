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

#include "../dom/excerpt.h"
#include "../dom/masterscore.h"
#include "../dom/part.h"
#include "../dom/score.h"
#include "../dom/staff.h"
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
