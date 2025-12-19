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

#include "editchord.h"

#include "../dom/arpeggio.h"
#include "../dom/chord.h"
#include "../dom/chordrest.h"
#include "../dom/factory.h"
#include "../dom/note.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/tremolotwochord.h"
#include "../dom/utils.h"

using namespace mu::engraving;

void EditChord::toggleChordParentheses(Chord* chord, std::vector<Note*> notes)
{
    bool hasParen = false;
    bool sameParenGroup = true;
    NoteParenthesisInfoList::iterator lastIt = EditChord::getChordParenIteratorFromNote(chord, notes.front());
    for (Note* n : notes) {
        NoteParenthesisInfoList::iterator noteIt = EditChord::getChordParenIteratorFromNote(chord, n);

        if (noteIt != chord->noteParens().end()) {
            hasParen = true;
            break;
        }

        if (noteIt != lastIt) {
            sameParenGroup = false;
            break;
        }
    }
    Parenthesis* leftParen = lastIt != chord->noteParens().end() ? lastIt->leftParen : nullptr;
    Parenthesis* rightParen = lastIt != chord->noteParens().end() ? lastIt->rightParen : nullptr;

    // Sort notes
    std::sort(notes.begin(), notes.end(), noteIsBefore);

    if (notes.size() == 1 && leftParen && rightParen) {
        // Remove paren from single note and create new paren group for all notes below
        Note* note = notes.front();
        std::vector<Note*>& notes = lastIt->notes;
        auto notePos = std::find(notes.begin(), notes.end(), note);

        if (notePos != notes.end() && notePos != notes.end() - 1) {
            // Create new group
            std::vector<Note*> newNoteGroup(notePos + 1, notes.end());

            for (Note* noteToRemove : newNoteGroup) {
                undoRemoveParenFromNote(chord, noteToRemove, leftParen, rightParen);
            }

            undoAddParensToNotes(chord, newNoteGroup);
        }
        undoRemoveParenFromNote(chord, note, leftParen, rightParen);
    } else if (!hasParen) {
        // Add parens to all notes in selection
        undoAddParensToNotes(chord, notes);
    } else if (sameParenGroup && leftParen && rightParen && lastIt->notes.size() == notes.size()) {
        // Remove parens from all notes in the group
        std::vector<Note*>& notesList = lastIt->notes;
        undoClearParenGroup(chord, notesList, leftParen, rightParen);
    }
}

NoteParenthesisInfoList::iterator EditChord::getChordParenIteratorFromParen(Chord* chord, Parenthesis* leftParen)
{
    NoteParenthesisInfoList::iterator foundIt = chord->noteParens().end();

    for (NoteParenthesisInfoList::iterator it = chord->noteParens().begin();
         it != chord->noteParens().end(); ++it) {
        auto& noteParenInfo = *it;

        if (noteParenInfo.leftParen == leftParen) {
            foundIt = it;
        }
    }

    DO_ASSERT_X(foundIt != chord->noteParens().end(), String(u"Parentheses are not in chord"));

    return foundIt;
}

NoteParenthesisInfoList::iterator EditChord::getChordParenIteratorFromNote(Chord* chord, Note* note)
{
    for (NoteParenthesisInfoList::iterator it = chord->noteParens().begin(); it != chord->noteParens().end(); ++it) {
        auto& noteParenInfo = *it;
        for (const Note* parenNote : noteParenInfo.notes) {
            if (parenNote == note) {
                return it;
            }
        }
    }

    return chord->noteParens().end();
}

void EditChord::undoAddParensToNotes(Chord* chord, std::vector<Note*> notes)
{
    track_idx_t track = chord->track();
    Score* score = chord->score();
    Parenthesis* leftParen = Factory::createParenthesis(chord);
    leftParen->setParent(chord);
    leftParen->setTrack(track);
    leftParen->setDirection(DirectionH::LEFT);
    Parenthesis* rightParen = Factory::createParenthesis(chord);
    rightParen->setParent(chord);
    rightParen->setTrack(track);
    rightParen->setDirection(DirectionH::RIGHT);

    for (EngravingObject* linkedObject : chord->linkList()) {
        if (linkedObject == chord) {
            score->undo(new AddNoteParentheses(chord, notes, leftParen, rightParen));
            continue;
        }

        Chord* linkedChord = toChord(linkedObject);
        Score* linkedScore = linkedChord->score();
        Parenthesis* linkedParenLeft = toParenthesis(leftParen->linkedClone());
        linkedParenLeft->setScore(linkedScore);
        linkedParenLeft->setParent(linkedChord);
        linkedParenLeft->setTrack(linkedChord->track());
        Parenthesis* linkedParenRight = toParenthesis(rightParen->linkedClone());
        linkedParenRight->setScore(linkedScore);
        linkedParenRight->setParent(linkedChord);
        linkedParenRight->setTrack(chord->track());

        std::vector<Note*> linkedNotes;
        for (Note* note : notes) {
            Note* linkedNote = toNote(note->findLinkedInScore(linkedScore));
            linkedNotes.push_back(linkedNote);
        }

        linkedScore->undo(new AddNoteParentheses(linkedChord, linkedNotes, linkedParenLeft, linkedParenRight));
    }
}

void EditChord::undoRemoveParenFromNote(Chord* chord, Note* note, Parenthesis* leftParen, Parenthesis* rightParen)
{
    Score* score = chord->score();
    for (EngravingObject* linkedObject : chord->linkList()) {
        if (linkedObject == chord) {
            score->undo(new RemoveSingleNoteParentheses(chord, note, leftParen, rightParen));
            continue;
        }

        Chord* linkedChord = toChord(linkedObject);
        Score* linkedScore = linkedChord->score();
        Parenthesis* linkedLeftParen = toParenthesis(leftParen->findLinkedInScore(linkedScore));
        Parenthesis* linkedRightParen = toParenthesis(rightParen->findLinkedInScore(linkedScore));
        Note* linkedNote = toNote(note->findLinkedInScore(linkedScore));

        linkedScore->undo(new RemoveSingleNoteParentheses(linkedChord, linkedNote, linkedLeftParen, linkedRightParen));
    }
}

void EditChord::undoClearParenGroup(Chord* chord, std::vector<Note*> notes, Parenthesis* leftParen, Parenthesis* rightParen)
{
    Score* score = chord->score();
    for (EngravingObject* linkedObject : chord->linkList()) {
        if (linkedObject == chord) {
            score->undo(new RemoveNoteParentheses(chord, notes, leftParen, rightParen));
            continue;
        }

        Chord* linkedChord = toChord(linkedObject);
        Score* linkedScore = linkedChord->score();
        Parenthesis* linkedLeftParen = toParenthesis(leftParen->findLinkedInScore(linkedScore));
        Parenthesis* linkedRightParen = toParenthesis(rightParen->findLinkedInScore(linkedScore));

        std::vector<Note*> linkedNotes;
        for (Note* note : notes) {
            Note* linkedNote = toNote(note->findLinkedInScore(linkedScore));
            linkedNotes.push_back(linkedNote);
        }

        linkedScore->undo(new RemoveNoteParentheses(linkedChord, linkedNotes, linkedLeftParen, linkedRightParen));
    }
}

//---------------------------------------------------------
//   ChangeChordStaffMove
//---------------------------------------------------------

void ChangeChordStaffMove::flip(EditData*)
{
    int v = chordRest->staffMove();
    staff_idx_t oldStaff = chordRest->vStaffIdx();

    chordRest->setStaffMove(staffMove);
    chordRest->checkStaffMoveValidity();
    chordRest->triggerLayout();
    if (chordRest->vStaffIdx() == oldStaff) {
        return;
    }

    for (EngravingObject* e : chordRest->linkList()) {
        ChordRest* cr = toChordRest(e);
        if (cr == chordRest) {
            continue;
        }
        cr->setStaffMove(staffMove);
        cr->checkStaffMoveValidity();
        cr->triggerLayout();
    }
    staffMove = v;
}

//---------------------------------------------------------
//   SwapCR
//---------------------------------------------------------

void SwapCR::flip(EditData*)
{
    Segment* s1 = cr1->segment();
    Segment* s2 = cr2->segment();
    track_idx_t track = cr1->track();

    if (cr1->isChord() && cr2->isChord() && toChord(cr1)->tremoloTwoChord()
        && (toChord(cr1)->tremoloTwoChord() == toChord(cr2)->tremoloTwoChord())) {
        TremoloTwoChord* t = toChord(cr1)->tremoloTwoChord();
        Chord* c1 = t->chord1();
        Chord* c2 = t->chord2();
        t->setParent(toChord(c2));
        t->setChords(toChord(c2), toChord(c1));
    }

    EngravingItem* cr = s1->element(track);
    s1->setElement(track, s2->element(track));
    s2->setElement(track, cr);
    cr1->score()->setLayout(s1->tick(), cr1->staffIdx(), cr1);
    cr1->score()->setLayout(s2->tick(), cr1->staffIdx(), cr1);
}

//---------------------------------------------------------
//   ChangeSpanArpeggio
//---------------------------------------------------------

void ChangeSpanArpeggio::flip(EditData*)
{
    Arpeggio* f_spanArp = m_chord->spanArpeggio();

    m_chord->setSpanArpeggio(m_spanArpeggio);
    m_spanArpeggio = f_spanArp;
}

void AddNoteParentheses::redo(EditData*)
{
    m_chord->noteParens().push_back(NoteParenthesisInfo(m_leftParen, m_rightParen, m_notes));
}

void AddNoteParentheses::undo(EditData*)
{
    NoteParenthesisInfoList::iterator foundIt = EditChord::getChordParenIteratorFromParen(m_chord, m_leftParen);

    m_chord->noteParens().erase(foundIt);

    m_chord->triggerLayout();
}

void RemoveNoteParentheses::redo(EditData*)
{
    NoteParenthesisInfoList::iterator foundIt = EditChord::getChordParenIteratorFromParen(m_chord, m_leftParen);

    m_chord->noteParens().erase(foundIt);

    m_chord->triggerLayout();
}

void RemoveNoteParentheses::undo(EditData*)
{
    m_chord->noteParens().push_back(NoteParenthesisInfo(m_leftParen, m_rightParen, m_notes));

    m_chord->triggerLayout();
}

void RemoveSingleNoteParentheses::redo(EditData*)
{
    NoteParenthesisInfoList::iterator foundIt = EditChord::getChordParenIteratorFromParen(m_chord, m_leftParen);

    std::vector<Note*>& notes = foundIt->notes;

    muse::remove(notes, m_note);

    m_chord->triggerLayout();
}

void RemoveSingleNoteParentheses::undo(EditData*)
{
    NoteParenthesisInfoList::iterator foundIt = EditChord::getChordParenIteratorFromParen(m_chord, m_leftParen);

    std::vector<Note*>& notes = foundIt->notes;

    notes.push_back(m_note);

    m_chord->triggerLayout();
}
