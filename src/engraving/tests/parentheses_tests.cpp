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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "engraving/compat/scoreaccess.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/note.h"
#include "engraving/dom/parenthesis.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu::engraving;

static const String PAREN_DATA(u"paren_data/");

class Engraving_ParenthesesTests : public ::testing::Test
{
protected:
    static void toggleSingleNoteParen(MasterScore* score, Note* note)
    {
        score->select(note);

        score->startCmd(TranslatableString::untranslatable("Parentheses tests"));
        score->cmdToggleParentheses();
        score->endCmd();
    }

    static void toggleMultipleNoteParen(MasterScore* score, std::vector<Note*>& notes)
    {
        std::vector<EngravingItem*> items(notes.begin(), notes.end());
        score->select(items, SelectType::ADD);

        score->startCmd(TranslatableString::untranslatable("Parentheses tests"));
        score->cmdToggleParentheses();
        score->endCmd();
    }

    static MasterScore* loadScore(const String& filename)
    {
        MasterScore* score = ScoreRW::readScore(PAREN_DATA + filename);
        EXPECT_TRUE(score);
        return score;
    }

    static Chord* findChordInMeasure(Measure* m, Fraction time, staff_idx_t staff)
    {
        Chord* chord = m->findChord(time, staff);
        EXPECT_TRUE(chord);
        return chord;
    }

    static void checkChordHasNoParens(Chord* chord)
    {
        EXPECT_TRUE(chord->noteParens().empty());
    }

    static void checkChordHasParens(Chord* chord, size_t count)
    {
        EXPECT_EQ(chord->noteParens().size(), count);
    }

    static void checkAllNotesHaveParenInfo(const std::vector<Note*>& notes)
    {
        for (const Note* note : notes) {
            EXPECT_TRUE(note->parenInfo());
        }
    }

    static void checkNoNotesHaveParenInfo(const std::vector<Note*>& notes)
    {
        for (const Note* note : notes) {
            EXPECT_FALSE(note->parenInfo());
        }
    }

    static void undoAndCheckRemoved(MasterScore* score, Chord* chord)
    {
        score->undoRedo(true, 0);
        checkChordHasNoParens(chord);
    }

    static void undoAndCheckRemovedLinked(MasterScore* score, Chord* chordStd, Chord* chordTab)
    {
        score->undoRedo(true, 0);
        checkChordHasNoParens(chordStd);
        checkChordHasNoParens(chordTab);
    }

    static void checkParenGroupsSplit(Chord* chord)
    {
        Note* n0 = chord->notes().at(0);
        Note* n1 = chord->notes().at(1);
        Note* n3 = chord->notes().at(3);
        Note* n4 = chord->notes().at(4);

        EXPECT_TRUE(n0->parenInfo() == n1->parenInfo());
        EXPECT_TRUE(n3->parenInfo() == n4->parenInfo());
        EXPECT_FALSE(n0->parenInfo() == n4->parenInfo());
    }
};

TEST_F(Engraving_ParenthesesTests, addParen)
{
    MasterScore* score = loadScore(u"single_staff.mscx");

    // Find note
    Measure* m1 = score->firstMeasure();
    Chord* singleNoteChord = findChordInMeasure(m1, Fraction(0, 1), 0);
    Note* note = singleNoteChord->notes().front();
    EXPECT_TRUE(note);

    checkChordHasNoParens(singleNoteChord);

    // Toggle parentheses
    toggleSingleNoteParen(score, note);

    checkChordHasParens(singleNoteChord, 1);

    // Check that paren info has been created
    EXPECT_TRUE(note->parenInfo());

    undoAndCheckRemoved(score, singleNoteChord);

    // Check that paren info has been removed
    EXPECT_FALSE(note->parenInfo());
}

TEST_F(Engraving_ParenthesesTests, addParenLinkedStaff)
{
    MasterScore* score = loadScore(u"linked_staff.mscx");

    // Find notes
    Measure* m1 = score->firstMeasure();
    Chord* singleNoteChordStd = findChordInMeasure(m1, Fraction(0, 1), 0);
    Chord* singleNoteChordTab = findChordInMeasure(m1, Fraction(0, 1), 4);
    Note* noteStd = singleNoteChordStd->notes().front();
    EXPECT_TRUE(noteStd);
    Note* noteTab = singleNoteChordTab->notes().front();
    EXPECT_TRUE(noteTab);

    checkChordHasNoParens(singleNoteChordStd);

    toggleSingleNoteParen(score, noteStd);

    checkChordHasParens(singleNoteChordStd, 1);
    checkChordHasParens(singleNoteChordTab, 1);

    EXPECT_TRUE(noteStd->parenInfo());
    EXPECT_TRUE(noteTab->parenInfo());

    EXPECT_TRUE(noteStd->parenInfo()->leftParen->isLinked(noteTab->parenInfo()->leftParen));
    EXPECT_TRUE(noteStd->parenInfo()->rightParen->isLinked(noteTab->parenInfo()->rightParen));

    undoAndCheckRemovedLinked(score, singleNoteChordStd, singleNoteChordTab);

    // Check that paren info has been removed
    EXPECT_FALSE(noteStd->parenInfo());
    EXPECT_FALSE(noteTab->parenInfo());
}

TEST_F(Engraving_ParenthesesTests, addParensManyNotes)
{
    MasterScore* score = loadScore(u"single_staff.mscx");

    // Find note
    Measure* m2 = score->firstMeasure()->nextMeasure();
    Chord* chord = findChordInMeasure(m2, Fraction(1, 1), 0);
    checkChordHasNoParens(chord);

    // Toggle parentheses
    toggleMultipleNoteParen(score, chord->notes());

    checkChordHasParens(chord, 1);

    // Check that paren info has been created
    checkAllNotesHaveParenInfo(chord->notes());

    undoAndCheckRemoved(score, chord);

    // Check that paren info has been removed
    checkNoNotesHaveParenInfo(chord->notes());
}

TEST_F(Engraving_ParenthesesTests, removeParensBottomNotes)
{
    MasterScore* score = loadScore(u"single_staff.mscx");

    // Find chord in second measure
    Measure* m2 = score->firstMeasure()->nextMeasure();
    Chord* chord = findChordInMeasure(m2, Fraction(1, 1), 0);
    checkChordHasNoParens(chord);

    // Toggle parentheses for all notes
    toggleMultipleNoteParen(score, chord->notes());
    checkChordHasParens(chord, 1);
    checkAllNotesHaveParenInfo(chord->notes());

    // Remove parentheses from bottom 2 notes
    std::vector<Note*> notes = chord->notes();
    std::vector<EngravingItem*> bottomNotes{ notes.at(notes.size() - 2), notes.at(notes.size() - 1) };
    score->select(bottomNotes, SelectType::ADD);
    score->startCmd(TranslatableString::untranslatable("Parentheses tests remove bottom notes"));
    score->cmdRemoveParenthesesFromNotes();
    score->endCmd();

    // Assert that these 2 notes have no paren info
    EXPECT_FALSE(notes.at(notes.size() - 2)->parenInfo());
    EXPECT_FALSE(notes.at(notes.size() - 1)->parenInfo());
}

TEST_F(Engraving_ParenthesesTests, addParensManyNotesLinkedStaff)
{
    MasterScore* score = loadScore(u"linked_staff.mscx");

    // Find note
    Measure* m2 = score->firstMeasure()->nextMeasure();
    Chord* chordStd = findChordInMeasure(m2, Fraction(1, 1), 0);
    checkChordHasNoParens(chordStd);
    Chord* chordTab = findChordInMeasure(m2, Fraction(1, 1), 4);
    checkChordHasNoParens(chordTab);

    // Toggle parentheses
    toggleMultipleNoteParen(score, chordStd->notes());

    checkChordHasParens(chordStd, 1);
    checkChordHasParens(chordTab, 1);

    // Check that paren info has been created
    checkAllNotesHaveParenInfo(chordStd->notes());
    checkAllNotesHaveParenInfo(chordTab->notes());

    undoAndCheckRemovedLinked(score, chordStd, chordTab);

    // Check that paren info has been removed
    checkNoNotesHaveParenInfo(chordStd->notes());
    checkNoNotesHaveParenInfo(chordTab->notes());
}

TEST_F(Engraving_ParenthesesTests, breakParenGroup)
{
    MasterScore* score = loadScore(u"single_staff.mscx");

    // Find note
    Measure* m2 = score->firstMeasure()->nextMeasure();
    Chord* chord = findChordInMeasure(m2, Fraction(1, 1), 0);
    checkChordHasNoParens(chord);

    // Toggle parentheses
    toggleMultipleNoteParen(score, chord->notes());

    checkChordHasParens(chord, 1);

    Note* middleNote = chord->notes().at(2);

    // Break parenthesis group in the middle
    toggleSingleNoteParen(score, middleNote);

    checkChordHasParens(chord, 2);
    EXPECT_FALSE(middleNote->parenInfo());

    Note* n0 = chord->notes().at(0);
    Note* n1 = chord->notes().at(1);
    Note* n3 = chord->notes().at(3);
    Note* n4 = chord->notes().at(4);

    EXPECT_TRUE(n0->parenInfo() == n1->parenInfo());
    EXPECT_TRUE(n3->parenInfo() == n4->parenInfo());
    EXPECT_FALSE(n0->parenInfo() == n4->parenInfo());

    // Undo, should have 1 parenthesis group again
    score->undoRedo(true, 0);

    checkChordHasParens(chord, 1);
    checkAllNotesHaveParenInfo(chord->notes());
}

TEST_F(Engraving_ParenthesesTests, breakParenGroupLinkedStaff)
{
    MasterScore* score = loadScore(u"linked_staff.mscx");

    // Find chords
    Measure* m2 = score->firstMeasure()->nextMeasure();
    Chord* chordStd = findChordInMeasure(m2, Fraction(1, 1), 0);
    checkChordHasNoParens(chordStd);
    Chord* chordTab = findChordInMeasure(m2, Fraction(1, 1), 4);
    checkChordHasNoParens(chordTab);

    // Toggle parentheses for standard staff
    toggleMultipleNoteParen(score, chordStd->notes());

    checkChordHasParens(chordStd, 1);
    checkChordHasParens(chordTab, 1);

    // Check that paren info has been created for both staves
    checkAllNotesHaveParenInfo(chordStd->notes());
    checkAllNotesHaveParenInfo(chordTab->notes());

    Note* middleNoteStd = chordStd->notes().at(2);

    // Break parenthesis group in the middle of standard staff
    toggleSingleNoteParen(score, middleNoteStd);

    checkChordHasParens(chordStd, 2);
    checkChordHasParens(chordTab, 2);
    EXPECT_FALSE(middleNoteStd->parenInfo());

    Note* middleNoteTab = chordTab->notes().at(2);
    EXPECT_FALSE(middleNoteTab->parenInfo());

    checkParenGroupsSplit(chordStd);
    checkParenGroupsSplit(chordTab);

    // Undo, should have 1 parenthesis group again in both staves
    score->undoRedo(true, 0);

    checkChordHasParens(chordStd, 1);
    checkChordHasParens(chordTab, 1);
    checkAllNotesHaveParenInfo(chordStd->notes());
    checkAllNotesHaveParenInfo(chordTab->notes());
}
