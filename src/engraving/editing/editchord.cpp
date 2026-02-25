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

void mu::engraving::EditChord::addChordParentheses(Chord* chord, std::vector<Note*> notes, bool addToLinked, bool generated)
{
    if (notes.empty()) {
        return;
    }
    undoAddParensToNotes(chord, notes, addToLinked, generated);
}

void mu::engraving::EditChord::removeChordParentheses(Chord* chord, std::vector<Note*> notes, bool addToLinked, bool generated)
{
    if (notes.empty()) {
        return;
    }
    // Split based on parentheses group
    std::map<const NoteParenthesisInfo*, std::vector<Note*> > notesByGroup;

    for (Note* n : notes) {
        const NoteParenthesisInfo* noteParenInfo = chord->findNoteParenInfo(n);
        if (!noteParenInfo) {
            continue;
        }

        auto groupIt = notesByGroup.find(noteParenInfo);

        if (groupIt == notesByGroup.end()) {
            std::vector<Note*> noteVec { n };
            notesByGroup.insert(std::make_pair(noteParenInfo, noteVec));
            continue;
        }

        groupIt->second.push_back(n);
    }

    for (auto pairIterator = notesByGroup.rbegin(); pairIterator != notesByGroup.rend(); pairIterator = std::next(pairIterator)) {
        auto groupNotesPair = *pairIterator;
        const NoteParenthesisInfo* parenInfo = groupNotesPair.first;
        if (parenInfo->notes.size() == groupNotesPair.second.size()) {
            // All notes marked for paren removal, clear parentheses group
            undoClearParenGroup(chord, parenInfo->notes, parenInfo->leftParen, parenInfo->rightParen, addToLinked);
            continue;
        }

        // Only some notes marked for removal. Remove from lowest note up. This keeps the paren group splitting logic safe
        for (auto noteIterator = groupNotesPair.second.begin(); noteIterator != groupNotesPair.second.end();
             noteIterator = std::next(noteIterator)) {
            // Remove paren from single note and create new paren group for all notes below
            Note* note = *noteIterator;
            const std::vector<Note*> notesList = parenInfo->notes;
            Parenthesis* leftParen = parenInfo->leftParen;
            Parenthesis* rightParen = parenInfo->rightParen;
            auto notePos = std::find(notesList.begin(), notesList.end(), note);

            if (notePos != notesList.end() && std::next(notePos) != notesList.end() && notePos != notesList.begin()) {
                // Create new group
                std::vector<Note*> newNoteGroup(notePos + 1, notesList.end());

                for (Note* noteToRemove : newNoteGroup) {
                    if (parenInfo->notes.size() == 1) {
                        undoClearParenGroup(chord, { noteToRemove }, leftParen, rightParen, addToLinked);
                    } else {
                        undoRemoveParenFromNote(chord, noteToRemove, leftParen, addToLinked);
                    }
                }

                undoAddParensToNotes(chord, newNoteGroup, addToLinked, generated);
            }
            if (parenInfo->notes.size() == 1) {
                undoClearParenGroup(chord, { note }, leftParen, rightParen, addToLinked);
            } else {
                undoRemoveParenFromNote(chord, note, leftParen, addToLinked);
            }
        }
    }
}

void EditChord::undoAddParensToNotes(Chord* chord, std::vector<Note*> notes, bool addToLinked, bool generated)
{
    track_idx_t track = chord->track();
    Parenthesis* leftParen = Factory::createParenthesis(chord);
    leftParen->setParent(chord);
    leftParen->setTrack(track);
    leftParen->setDirection(DirectionH::LEFT);
    leftParen->setGenerated(generated);
    Parenthesis* rightParen = Factory::createParenthesis(chord);
    rightParen->setParent(chord);
    rightParen->setTrack(track);
    rightParen->setDirection(DirectionH::RIGHT);
    rightParen->setGenerated(generated);

    if (!addToLinked) {
        doAddNoteParentheses(chord, notes, leftParen, rightParen);
        return;
    }

    for (EngravingObject* linkedObject : chord->linkList()) {
        if (linkedObject == chord) {
            doAddNoteParentheses(chord, notes, leftParen, rightParen);
            continue;
        }

        Chord* linkedChord = toChord(linkedObject);
        Score* linkedScore = linkedChord->score();
        Staff* linkedStaff = linkedChord->staff();
        Parenthesis* linkedParenLeft = toParenthesis(leftParen->linkedClone());
        linkedParenLeft->setScore(linkedScore);
        linkedParenLeft->setParent(linkedChord);
        linkedParenLeft->setTrack(linkedChord->track());
        Parenthesis* linkedParenRight = toParenthesis(rightParen->linkedClone());
        linkedParenRight->setScore(linkedScore);
        linkedParenRight->setParent(linkedChord);
        linkedParenRight->setTrack(linkedChord->track());

        std::vector<Note*> linkedNotes;
        for (Note* note : notes) {
            Note* linkedNote = toNote(note->findLinkedInStaff(linkedStaff));
            linkedNotes.push_back(linkedNote);
        }

        doAddNoteParentheses(linkedChord, linkedNotes, linkedParenLeft, linkedParenRight);
    }
}

void EditChord::undoRemoveParenFromNote(Chord* chord, Note* note, Parenthesis* leftParen, bool removeFromLinked)
{
    if (!removeFromLinked) {
        doRemoveSingleNoteParen(chord, note, leftParen);
        return;
    }

    for (EngravingObject* linkedObject : chord->linkList()) {
        if (linkedObject == chord) {
            doRemoveSingleNoteParen(chord, note, leftParen);
            continue;
        }

        Chord* linkedChord = toChord(linkedObject);
        Staff* linkedStaff = linkedChord->staff();
        Parenthesis* linkedLeftParen = toParenthesis(leftParen->findLinkedInStaff(linkedStaff));
        if (!linkedLeftParen) {
            continue;
        }
        Note* linkedNote = toNote(note->findLinkedInStaff(linkedStaff));

        const NoteParenthesisInfo* noteParenInfo = linkedChord->findNoteParenInfo(linkedNote);
        if (noteParenInfo) {
            doRemoveSingleNoteParen(linkedChord, linkedNote, linkedLeftParen);
        }
    }
}

void EditChord::undoClearParenGroup(Chord* chord, std::vector<Note*> notes, Parenthesis* leftParen, Parenthesis* rightParen,
                                    bool removeFromLinked)
{
    if (!removeFromLinked) {
        doRemoveAllNoteParens(chord, notes, leftParen, rightParen);
        return;
    }

    for (EngravingObject* linkedObject : chord->linkList()) {
        if (linkedObject == chord) {
            doRemoveAllNoteParens(chord, notes, leftParen, rightParen);
            continue;
        }

        Chord* linkedChord = toChord(linkedObject);
        Staff* linkedStaff = linkedChord->staff();
        Parenthesis* linkedLeftParen = toParenthesis(leftParen->findLinkedInStaff(linkedStaff));
        Parenthesis* linkedRightParen = toParenthesis(rightParen->findLinkedInStaff(linkedStaff));

        if (!linkedLeftParen && !linkedRightParen) {
            continue;
        }

        std::vector<Note*> linkedNotes;
        for (Note* note : notes) {
            Note* linkedNote = toNote(note->findLinkedInStaff(linkedStaff));
            linkedNotes.push_back(linkedNote);
        }

        doRemoveAllNoteParens(linkedChord, linkedNotes, linkedLeftParen, linkedRightParen);
    }
}

void EditChord::doAddNoteParentheses(Chord* chord, std::vector<Note*> notes, Parenthesis* leftParen, Parenthesis* rightParen)
{
    if (leftParen->generated()) {
        chord->addNoteParenInfo(leftParen, rightParen, notes);
    } else {
        Score* score = chord->score();
        score->undo(new AddNoteParentheses(chord, notes, leftParen, rightParen));
    }
}

void EditChord::doRemoveSingleNoteParen(Chord* chord, Note* note, Parenthesis* leftParen)
{
    if (leftParen->generated()) {
        chord->removeNoteFromParenInfo(note, leftParen);

        chord->triggerLayout();
    } else {
        Score* score = chord->score();
        score->undo(new RemoveSingleNoteParentheses(chord, note, leftParen));
    }
}

void EditChord::doRemoveAllNoteParens(Chord* chord, std::vector<Note*> notes, Parenthesis* leftParen, Parenthesis* rightParen)
{
    if (leftParen->generated()) {
        const NoteParenthesisInfo* noteParenInfo = chord->findNoteParenInfo(leftParen);

        chord->removeNoteParenInfo(noteParenInfo);

        chord->triggerLayout();
    } else {
        Score* score = chord->score();
        score->undo(new RemoveNoteParentheses(chord, notes, leftParen, rightParen));
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
    m_chord->addNoteParenInfo(m_leftParen, m_rightParen, m_notes);
}

void AddNoteParentheses::undo(EditData*)
{
    const NoteParenthesisInfo* noteParenInfo = m_chord->findNoteParenInfo(m_leftParen);

    m_chord->removeNoteParenInfo(noteParenInfo);

    m_chord->triggerLayout();
}

void RemoveNoteParentheses::redo(EditData*)
{
    const NoteParenthesisInfo* noteParenInfo = m_chord->findNoteParenInfo(m_leftParen);

    m_chord->removeNoteParenInfo(noteParenInfo);

    m_chord->triggerLayout();
}

void RemoveNoteParentheses::undo(EditData*)
{
    m_chord->addNoteParenInfo(m_leftParen, m_rightParen, m_notes);

    m_chord->triggerLayout();
}

void RemoveSingleNoteParentheses::redo(EditData*)
{
    m_chord->removeNoteFromParenInfo(m_note, m_paren);

    m_chord->triggerLayout();
}

void RemoveSingleNoteParentheses::undo(EditData*)
{
    m_chord->addNoteToParenInfo(m_note, m_paren);

    m_chord->triggerLayout();
}
