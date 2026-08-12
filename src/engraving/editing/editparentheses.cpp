/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "editparentheses.h"

#include <algorithm>
#include <map>
#include <set>

#include "editchord.h"

#include "../dom/accidental.h"
#include "../dom/chord.h"
#include "../dom/engravingitem.h"
#include "../dom/note.h"
#include "../dom/score.h"
#include "../dom/select.h"
#include "../dom/timesig.h"
#include "../dom/utils.h"

#include "log.h"

using namespace mu::engraving;

namespace {
struct NoteComparator {
    bool operator()(const Note* n1, const Note* n2) const
    {
        if (noteIsBefore(n1, n2)) {
            return true;
        }
        if (noteIsBefore(n2, n1)) {
            return false;
        }
        // tiebreak by pointer address so we can have unison notes in the set
        return n1 < n2;
    }
};

static std::map<Chord*, std::set<Note*, NoteComparator> > getNotesByChord(const std::vector<Note*>& notes)
{
    // Return map of notes by chord
    // Include all notes between highest and lowest in each chord
    std::map<Chord*, std::set<Note*, NoteComparator> > notesByChord;
    for (Note* noteToAdd : notes) {
        Chord* chord = noteToAdd->chord();
        auto notesByChordIt = notesByChord.find(chord);

        if (notesByChordIt == notesByChord.end()) {
            notesByChord.emplace(chord, std::set<Note*, NoteComparator> { noteToAdd });
            continue;
        }

        // Add all notes between last note and this one
        const Note* prevNote = *notesByChordIt->second.rbegin();

        const Note* firstNote = noteIsBefore(noteToAdd, prevNote) ? noteToAdd : prevNote;
        const Note* secondNote = firstNote == noteToAdd ? prevNote : noteToAdd;

        const std::vector<Note*>& chordNotes = chord->notes();

        std::vector<Note*>::const_iterator firstNoteIt = std::find(chordNotes.begin(), chordNotes.end(), firstNote);
        std::vector<Note*>::const_iterator secondNoteIt = std::find(chordNotes.begin(), chordNotes.end(), secondNote);

        assert(firstNoteIt != chordNotes.end() && secondNoteIt != chordNotes.end());

        for (std::vector<Note*>::const_iterator chordNoteIt = firstNoteIt; chordNoteIt != std::next(secondNoteIt);
             chordNoteIt = std::next(chordNoteIt)) {
            Note* note = *chordNoteIt;
            notesByChordIt->second.insert(note);
        }
    }

    return notesByChord;
}
}

void EditParentheses::toggleParenthesesOnNotes(Transaction& tx, Score* score)
{
    std::vector<Note*> notes = score->selection().uniqueNotes(muse::nidx, false);

    bool add = false;

    for (Note* note : notes) {
        if (note->getProperty(Pid::HAS_PARENTHESES).value<ParenthesesMode>() == ParenthesesMode::NONE) {
            add = true;
            break;
        }
    }

    if (add) {
        addParenthesesToNotes(tx, notes);
    } else {
        removeParenthesesFromNotes(tx, notes);
    }
}

void EditParentheses::addParenthesesToNotes(Transaction&, const std::vector<Note*>& notes)
{
    std::map<Chord*, std::set<Note*, NoteComparator> > notesByChord = getNotesByChord(notes);

    for (auto& [chord, noteSet] : notesByChord) {
        std::vector<Note*> noteVec(noteSet.begin(), noteSet.end());

        for (Note* note : noteVec) {
            // User has overriden generated parentheses
            note->undoChangeProperty(Pid::HIDE_GENERATED_PARENTHESES, true);
            note->undoChangeProperty(Pid::HAS_PARENTHESES, ParenthesesMode::BOTH);
        }

        EditChord::removeChordParentheses(chord, noteVec);
        EditChord::addChordParentheses(chord, std::move(noteVec));
    }
}

void EditParentheses::addParenthesesToNotes(Transaction& tx, Score* score)
{
    addParenthesesToNotes(tx, score->selection().uniqueNotes(muse::nidx, false));
}

void EditParentheses::removeParenthesesFromNotes(Transaction&, const std::vector<Note*>& notes)
{
    std::map<Chord*, std::set<Note*, NoteComparator> > notesByChord = getNotesByChord(notes);

    for (auto& [chord, noteSet] : notesByChord) {
        std::vector<Note*> noteVec(noteSet.begin(), noteSet.end());

        for (Note* note : noteVec) {
            note->undoChangeProperty(Pid::HAS_PARENTHESES, ParenthesesMode::NONE);

            // User has overriden generated parentheses
            note->undoChangeProperty(Pid::HIDE_GENERATED_PARENTHESES, true);
        }

        EditChord::removeChordParentheses(chord, std::move(noteVec));
    }
}

void EditParentheses::removeParenthesesFromNotes(Transaction& tx, Score* score)
{
    removeParenthesesFromNotes(tx, score->selection().uniqueNotes(muse::nidx, false));
}

void EditParentheses::toggleParentheses(Transaction& tx, Score* score)
{
    toggleParenthesesOnNotes(tx, score);

    for (EngravingItem* el : score->selection().elements()) {
        if (el->isNote()) {
            continue;
        }
        toggleParenthesesOnItem(tx, el);
    }
}

void EditParentheses::toggleParenthesesOnItem(Transaction&, EngravingItem* el)
{
    if (el->isAccidental()) {
        Accidental* acc = toAccidental(el);
        acc->undoChangeProperty(Pid::ACCIDENTAL_BRACKET, int(AccidentalBracket::PARENTHESIS));
    } else if (el->type() == ElementType::TIMESIG) {
        TimeSig* ts = toTimeSig(el);
        ts->setLargeParentheses(true);
    } else {
        ParenthesesMode p = el->leftParen() || el->rightParen() ? ParenthesesMode::NONE : ParenthesesMode::BOTH;
        el->undoChangeProperty(Pid::HAS_PARENTHESES, p);
    }
}
