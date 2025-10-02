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

#pragma once

#include "undo.h"

#include "../dom/note.h"

namespace mu::engraving {
class ChangePitch : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangePitch)

    Note* note = nullptr;
    int pitch = 0;
    int tpc1 = 0;
    int tpc2 = 0;

    void flip(EditData*) override;

public:
    ChangePitch(Note* note, int pitch, int tpc1, int tpc2);

    UNDO_TYPE(CommandType::ChangePitch)
    UNDO_NAME("ChangePitch")
    UNDO_CHANGED_OBJECTS({ note })
};

class ChangeFretting : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeFretting)

    Note* note = nullptr;
    int pitch = 0;
    int string = 0;
    int fret = 0;
    int tpc1 = 0;
    int tpc2 = 0;

    void flip(EditData*) override;

public:
    ChangeFretting(Note* note, int pitch, int string, int fret, int tpc1, int tpc2);

    UNDO_TYPE(CommandType::ChangeFretting)
    UNDO_NAME("ChangeFretting")
    UNDO_CHANGED_OBJECTS({ note })
};

class ChangeVelocity : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeVelocity)

    Note* note = nullptr;
    int userVelocity = 0;

    void flip(EditData*) override;

public:
    ChangeVelocity(Note*, int);

    UNDO_TYPE(CommandType::ChangeVelocity)
    UNDO_NAME("ChangeVelocity")
    UNDO_CHANGED_OBJECTS({ note })
};

class ChangeNoteEventList : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeNoteEventList)

    Note* note = nullptr;
    NoteEventList newEvents;
    PlayEventType newPetype;

    void flip(EditData*) override;

public:
    ChangeNoteEventList(Note* n, NoteEventList& ne)
        : note(n), newEvents(ne), newPetype(PlayEventType::User) {}
    UNDO_NAME("ChangeNoteEventList")
    UNDO_CHANGED_OBJECTS({ note });
};

class ChangeNoteEvent : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeNoteEvent)

    Note* note = nullptr;
    NoteEvent* oldEvent = nullptr;
    NoteEvent newEvent;
    PlayEventType newPetype;

    void flip(EditData*) override;

public:
    ChangeNoteEvent(Note* n, NoteEvent* oe, const NoteEvent& ne)
        : note(n), oldEvent(oe), newEvent(ne), newPetype(PlayEventType::User) {}
    UNDO_NAME("ChangeNoteEvent")
    UNDO_CHANGED_OBJECTS({ note })
};

class ChangeChordPlayEventType : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeChordPlayEventType)

    Chord* chord = nullptr;
    PlayEventType petype;
    std::vector<NoteEventList> events;

    void flip(EditData*) override;

public:
    ChangeChordPlayEventType(Chord* c, PlayEventType pet)
        : chord(c), petype(pet)
    {
        events = c->getNoteEventLists();
    }

    UNDO_NAME("ChangeChordPlayEventType")
    UNDO_CHANGED_OBJECTS({ chord });
};
}
