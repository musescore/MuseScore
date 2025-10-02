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

#include "editnote.h"

#include "dom/chord.h"
#include "dom/note.h"
#include "dom/score.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   ChangePitch
//---------------------------------------------------------

ChangePitch::ChangePitch(Note* _note, int _pitch, int _tpc1, int _tpc2)
{
    note  = _note;
    pitch = _pitch;
    tpc1  = _tpc1;
    tpc2  = _tpc2;
}

void ChangePitch::flip(EditData*)
{
    int f_pitch = note->pitch();
    int f_tpc1  = note->tpc1();
    int f_tpc2  = note->tpc2();
    // do not change unless necessary
    if (f_pitch == pitch && f_tpc1 == tpc1 && f_tpc2 == tpc2) {
        return;
    }

    note->setPitch(pitch, tpc1, tpc2);
    pitch = f_pitch;
    tpc1  = f_tpc1;
    tpc2  = f_tpc2;

    note->triggerLayout();
}

//---------------------------------------------------------
//   ChangeFretting
//
//    To use with tablatures to force a specific note fretting;
//    Pitch, string and fret must be changed all together; otherwise,
//    if they are not consistent among themselves, the refretting algorithm may re-assign
//    fret and string numbers for (potentially) all the notes of all the chords of a segment.
//---------------------------------------------------------

ChangeFretting::ChangeFretting(Note* _note, int _pitch, int _string, int _fret, int _tpc1, int _tpc2)
{
    note  = _note;
    pitch = _pitch;
    string= _string;
    fret  = _fret;
    tpc1  = _tpc1;
    tpc2  = _tpc2;
}

void ChangeFretting::flip(EditData*)
{
    int f_pitch = note->pitch();
    int f_string= note->string();
    int f_fret  = note->fret();
    int f_tpc1  = note->tpc1();
    int f_tpc2  = note->tpc2();
    // do not change unless necessary
    if (f_pitch == pitch && f_string == string && f_fret == fret && f_tpc1 == tpc1 && f_tpc2 == tpc2) {
        return;
    }

    note->setPitch(pitch, tpc1, tpc2);
    note->setString(string);
    note->setFret(fret);
    pitch = f_pitch;
    string= f_string;
    fret  = f_fret;
    tpc1  = f_tpc1;
    tpc2  = f_tpc2;
    note->triggerLayout();
}

//---------------------------------------------------------
//   ChangeVelocity
//---------------------------------------------------------

ChangeVelocity::ChangeVelocity(Note* n, int o)
    : note(n), userVelocity(o)
{
}

void ChangeVelocity::flip(EditData*)
{
    int v = note->userVelocity();
    note->setUserVelocity(userVelocity);
    userVelocity = v;
}

//---------------------------------------------------------
//   ChangeNoteEventList::flip
//---------------------------------------------------------

void ChangeNoteEventList::flip(EditData*)
{
    note->score()->setPlaylistDirty();
    // Get copy of current list.
    NoteEventList nel = note->playEvents();
    // Replace current copy with new list.
    note->setPlayEvents(newEvents);
    // Save copy of replaced list.
    newEvents = nel;
    // Get a copy of the current playEventType.
    PlayEventType petval = note->chord()->playEventType();
    // Replace current setting with new setting.
    note->chord()->setPlayEventType(newPetype);
    // Save copy of old setting.
    newPetype = petval;
}

//---------------------------------------------------------
//   ChangeNoteEvent::flip
//---------------------------------------------------------

void ChangeNoteEvent::flip(EditData*)
{
    note->score()->setPlaylistDirty();
    NoteEvent e = *oldEvent;
    *oldEvent   = newEvent;
    newEvent    = e;
    // Get a copy of the current playEventType.
    PlayEventType petval = note->chord()->playEventType();
    // Replace current setting with new setting.
    note->chord()->setPlayEventType(newPetype);
    // Save copy of old setting.
    newPetype = petval;
}

//---------------------------------------------------------
//   ChangeChordPlayEventType::flip
//---------------------------------------------------------

void ChangeChordPlayEventType::flip(EditData*)
{
    chord->score()->setPlaylistDirty();
    // Flips data between NoteEventList's.
    size_t n = chord->notes().size();
    for (size_t i = 0; i < n; ++i) {
        Note* note = chord->notes()[i];
        note->playEvents().swap(events[int(i)]);
    }
    // Flips PlayEventType between chord and undo.
    PlayEventType curPetype = chord->playEventType();
    chord->setPlayEventType(petype);
    petype = curPetype;
}
