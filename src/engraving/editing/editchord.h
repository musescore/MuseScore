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
#include "../dom/parenthesis.h"

namespace mu::engraving {
class EditChord
{
public:
    static void addChordParentheses(Chord* chord, std::vector<Note*> notes, bool addToLinked = true, bool generated = false);
    static void removeChordParentheses(Chord* chord, std::vector<Note*> notes, bool addToLinked = true, bool generated = false);

private:

    static void undoAddParensToNotes(Chord* chord, std::vector<Note*> notes, bool addToLinked = true, bool generated = false);
    static void undoRemoveParenFromNote(Chord* chord, Note* note, Parenthesis* leftParen, bool removeFromLinked = true);
    static void undoClearParenGroup(Chord* chord, std::vector<Note*> notes, Parenthesis* leftParen, Parenthesis* rightParen,
                                    bool removeFromLinked = true);

    static void doAddNoteParentheses(Chord* chord, std::vector<Note*> notes, Parenthesis* leftParen, Parenthesis* rightParen);
    static void doRemoveSingleNoteParen(Chord* chord, Note* note, Parenthesis* leftParen);
    static void doRemoveAllNoteParens(Chord* chord, Parenthesis* leftParen);
};

class ChangeChordStaffMove : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeChordStaffMove)

    ChordRest* chordRest = nullptr;
    int staffMove = 0;

    void flip(EditData*) override;

public:
    ChangeChordStaffMove(ChordRest* cr, int v)
        : chordRest(cr), staffMove(v) {}

    UNDO_TYPE(CommandType::ChangeChordStaffMove)
    UNDO_NAME("ChangeChordStaffMove")
    UNDO_CHANGED_OBJECTS({ chordRest })
};

class SwapCR : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, SwapCR)

    ChordRest* cr1 = nullptr;
    ChordRest* cr2 = nullptr;

    void flip(EditData*) override;

public:
    SwapCR(ChordRest* a, ChordRest* b)
        : cr1(a), cr2(b) {}

    UNDO_TYPE(CommandType::SwapCR)
    UNDO_NAME("SwapCR")
    UNDO_CHANGED_OBJECTS({ cr1, cr2 })
};

class ChangeSpanArpeggio : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeSpanArpeggio)

    Chord* m_chord = nullptr;
    Arpeggio* m_spanArpeggio = nullptr;

    void flip(EditData*) override;
public:
    ChangeSpanArpeggio(Chord* chord, Arpeggio* spanArp)
        : m_chord(chord), m_spanArpeggio(spanArp) {}

    UNDO_NAME("ChangeSpanArpeggio")
    UNDO_CHANGED_OBJECTS({ m_chord })
};

class AddNoteParenthesisInfo : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, AddNoteParenthesisInfo)

    void redo(EditData*) override;
    void undo(EditData*) override;

    void cleanup(bool undo) override;

    Chord* m_chord = nullptr;
    NoteParenthesisInfo* m_noteParenInfo = nullptr;

public:
    AddNoteParenthesisInfo(Chord* chord, NoteParenthesisInfo* noteParenInfo)
        : m_chord(chord), m_noteParenInfo(noteParenInfo) {}

    UNDO_NAME("AddNoteParenthesesInfo")
    UNDO_TYPE(CommandType::AddNoteParenthesesInfo)
};

class RemoveNoteParenthesisInfo : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveNoteParenthesisInfo)

    void redo(EditData*) override;
    void undo(EditData*) override;

    void cleanup(bool undo) override;

    Chord* m_chord = nullptr;
    NoteParenthesisInfo* m_noteParenInfo = nullptr;

public:
    RemoveNoteParenthesisInfo(Chord* chord, NoteParenthesisInfo* noteParenInfo)
        : m_chord(chord), m_noteParenInfo(noteParenInfo) {}

    UNDO_NAME("RemoveNoteParenthesesInfo")
    UNDO_TYPE(CommandType::RemoveNoteParenthesesInfo)
};

class RemoveSingleNoteParentheses : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveSingleNoteParentheses)

    Chord* m_chord = nullptr;
    Note* m_note = nullptr;
    Parenthesis* m_paren = nullptr;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    RemoveSingleNoteParentheses(Chord* chord, Note* note, Parenthesis* paren)
        : m_chord(chord), m_note(note), m_paren(paren) {}

    UNDO_NAME("RemoveSingleNoteParentheses")
    UNDO_TYPE(CommandType::RemoveSingleNoteParentheses)
    UNDO_CHANGED_OBJECTS({ m_chord, m_paren })
};
}
