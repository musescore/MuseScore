/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "libmscore/measure.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/rest.h"
#include "libmscore/stem.h"
#include "libmscore/hook.h"
#include "libmscore/beam.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String CHANGEVISIBILITY_DATA_DIR(u"changevisibility_data/");

class Engraving_ChangeVisibilityTests : public ::testing::Test
{
protected:
    std::vector<EngravingItem*> collectChildren(const Chord* chord) const
    {
        std::vector<EngravingItem*> children;

        for (EngravingObject* obj : chord->scanChildren()) {
            if (obj->isEngravingItem()) {
                children.push_back(toEngravingItem(obj));
            }
        }

        return children;
    }

    std::vector<Chord*> collectChords(const Beam* beam) const
    {
        std::vector<Chord*> chords;

        for (EngravingItem* item : beam->elements()) {
            if (item->isChord()) {
                chords.push_back(toChord(item));
            }
        }

        return chords;
    }

    Chord* findChord(const Score* score, int tick) const
    {
        ChordRest* cr = score->findCR(Fraction::fromTicks(tick), 0);
        if (cr->isChord()) {
            return toChord(cr);
        }

        return nullptr;
    }

    Rest* findRest(const Score* score, int tick) const
    {
        ChordRest* cr = score->findCR(Fraction::fromTicks(tick), 0);
        if (cr->isRest()) {
            return toRest(cr);
        }

        return nullptr;
    }
};

TEST_F(Engraving_ChangeVisibilityTests, UndoChangeVisible_SingleNoteChord)
{
    MasterScore* score = ScoreRW::readScore(CHANGEVISIBILITY_DATA_DIR + u"changevisibility.mscx");
    ASSERT_TRUE(score);

    // [GIVEN] Chord containing only one note
    Chord* chord = findChord(score, 0);
    ASSERT_TRUE(chord);

    ASSERT_TRUE(chord->notes().size() == 1);
    Note* note = chord->notes().front();
    ASSERT_TRUE(note);

    // [GIVEN] All items attached to this chord
    std::vector<EngravingItem*> children = collectChildren(chord);
    ASSERT_FALSE(children.empty());

    // [WHEN] Hide the note
    score->undoChangeVisible(note, false);

    // [THEN] Everything in the chord is hidden
    for (EngravingItem* child : children) {
        EXPECT_FALSE(child->visible());
    }

    // [WHEN] Show the note
    score->undoChangeVisible(note, true);

    // [THEN] Everything in the chord is visible
    for (EngravingItem* child : children) {
        EXPECT_TRUE(child->visible());
    }

    // [GIVEN] Some parts of the note
    ASSERT_FALSE(note->dots().empty());
    NoteDot* dot = note->dots().front();

    Stem* stem = chord->stem();
    ASSERT_TRUE(stem);

    // [WHEN] We can also hide the parts of the note
    score->undoChangeVisible(dot, false);
    score->undoChangeVisible(stem, false);

    // [THEN] Everything in the chord is visible, except the parts that were hidden manually
    for (EngravingItem* child : children) {
        if (child == dot || child == stem) {
            EXPECT_FALSE(child->visible());
        } else {
            EXPECT_TRUE(child->visible());
        }
    }

    delete score;
}

TEST_F(Engraving_ChangeVisibilityTests, UndoChangeVisible_RestWithDot)
{
    MasterScore* score = ScoreRW::readScore(CHANGEVISIBILITY_DATA_DIR + u"changevisibility.mscx");
    ASSERT_TRUE(score);

    // [GIVEN] Rest with a dot
    Rest* rest = findRest(score, 480);
    ASSERT_TRUE(rest);

    ASSERT_TRUE(!rest->dotList().empty());
    NoteDot* dot = rest->dotList().front();

    // [WHEN] Hide the rest
    score->undoChangeVisible(rest, false);

    // [THEN] Rest and its dot are hidden
    EXPECT_FALSE(rest->visible());
    EXPECT_FALSE(dot->visible());

    // [WHEN] Show the rest
    score->undoChangeVisible(rest, true);

    // [THEN] Rest and its dot are visible
    EXPECT_TRUE(rest->visible());
    EXPECT_TRUE(dot->visible());

    // [WHEN] We can also hide the dot
    score->undoChangeVisible(dot, false);

    // [THEN] Rest is visible, but the dot is not
    EXPECT_TRUE(rest->visible());
    EXPECT_FALSE(dot->visible());

    delete score;
}

TEST_F(Engraving_ChangeVisibilityTests, UndoChangeVisible_ChordContainingSeveralNotes)
{
    MasterScore* score = ScoreRW::readScore(CHANGEVISIBILITY_DATA_DIR + u"changevisibility.mscx");
    ASSERT_TRUE(score);

    // [GIVEN] Chord containing several notes
    Chord* chord = findChord(score, 1920);
    ASSERT_TRUE(chord);
    ASSERT_TRUE(chord->notes().size() > 2);

    // [GIVEN] Parts of the chord
    Stem* stem = chord->stem();
    ASSERT_TRUE(stem);

    Hook* hook = chord->hook();
    ASSERT_TRUE(hook);

    std::vector<Articulation*> arcticulations = chord->articulations();
    ASSERT_FALSE(arcticulations.empty());

    // [GIVEN] First note
    Note* firstNote = chord->notes().front();
    ASSERT_TRUE(firstNote);

    // [WHEN] Hide the first note
    score->undoChangeVisible(firstNote, false);

    // [THEN] Only the first note is hidden
    EXPECT_FALSE(firstNote->visible());

    for (const Note* note : chord->notes()) {
        if (note == firstNote) {
            EXPECT_FALSE(note->visible());
        } else {
            EXPECT_TRUE(note->visible());
        }
    }

    EXPECT_TRUE(stem->visible());
    EXPECT_TRUE(hook->visible());

    for (const Articulation* articulation : arcticulations) {
        EXPECT_TRUE(articulation->visible());
    }

    // [GIVEN] Second note
    Note* secondNote = chord->notes().front();
    ASSERT_TRUE(secondNote);

    // [WHEN] Hide the second note
    score->undoChangeVisible(firstNote, false);

    // [THEN] Only the first and the second notes are hidden
    EXPECT_FALSE(firstNote->visible());

    for (const Note* note : chord->notes()) {
        if (note == firstNote || note == secondNote) {
            EXPECT_FALSE(note->visible());
        } else {
            EXPECT_TRUE(note->visible());
        }
    }

    EXPECT_TRUE(stem->visible());
    EXPECT_TRUE(hook->visible());

    // [WHEN] Hide all notes
    for (Note* note : chord->notes()) {
        score->undoChangeVisible(note, false);
    }

    // [GIVEN] All items attached to this chord
    std::vector<EngravingItem*> children = collectChildren(chord);
    ASSERT_FALSE(children.empty());

    // [THEN] Everything in the chord is now hidden (including the stem and the hook)
    for (EngravingItem* child : children) {
        EXPECT_FALSE(child->visible());
    }

    EXPECT_FALSE(stem->visible());
    EXPECT_FALSE(hook->visible());

    delete score;
}

TEST_F(Engraving_ChangeVisibilityTests, UndoChangeVisible_ChordsConnectedWithBeam)
{
    MasterScore* score = ScoreRW::readScore(CHANGEVISIBILITY_DATA_DIR + u"changevisibility.mscx");
    ASSERT_TRUE(score);

    // [GIVEN] First chord under the beam
    Chord* firstChord = findChord(score, 3840);
    ASSERT_TRUE(firstChord);
    ASSERT_TRUE(!firstChord->notes().empty());

    // [GIVEN] Stem of the first chord
    Stem* firstChordStem = firstChord->stem();
    ASSERT_TRUE(firstChordStem);

    // [GIVEN] Beam and its chords
    Beam* beam = firstChord->beam();
    ASSERT_TRUE(beam);

    std::vector<Chord*> chordsConnectedWithBeam = collectChords(beam);
    ASSERT_FALSE(chordsConnectedWithBeam.empty());

    // [WHEN] Hide all notes in the first chord
    for (Note* note : firstChord->notes()) {
        score->undoChangeVisible(note, false);
    }

    // [THEN] Beam/Steam is visible
    EXPECT_TRUE(beam->visible());
    EXPECT_TRUE(firstChordStem->visible());

    for (Chord* chord : chordsConnectedWithBeam) {
        std::vector<EngravingItem*> children = collectChildren(chord);
        ASSERT_TRUE(!children.empty());

        for (const EngravingItem* child : children) {
            // [THEN] Stems/Beam remain visible
            if (child->isStem() || child->isBeam()) {
                EXPECT_TRUE(child->visible());
                continue;
            }

            // [THEN] Everything in the first chord is hidden
            if (chord == firstChord) {
                EXPECT_FALSE(child->visible());
            } else {
                EXPECT_TRUE(child->visible());
            }
        }
    }

    // [WHEN] Hide all notes of all chords under the beam
    for (Chord* chord : chordsConnectedWithBeam) {
        std::vector<EngravingItem*> children = collectChildren(chord);

        for (EngravingItem* child : children) {
            if (child->isNote()) {
                score->undoChangeVisible(child, false);
            }
        }
    }

    // [THEN] Everything under the beam is hidden (including the beam itself)
    EXPECT_FALSE(beam->visible());

    for (Chord* chord : chordsConnectedWithBeam) {
        std::vector<EngravingItem*> children = collectChildren(chord);
        ASSERT_TRUE(!children.empty());

        for (const EngravingItem* child : children) {
            EXPECT_FALSE(child->visible());
        }
    }

    // [GIVEN] First note of the first chord
    Note* firstChordNote = firstChord->notes().front();

    // [WHEN] Show it
    score->undoChangeVisible(firstChordNote, true);

    // [THEN] All stems/beam are visible now
    EXPECT_TRUE(beam->visible());
    EXPECT_TRUE(firstChordNote->visible());

    for (Chord* chord : chordsConnectedWithBeam) {
        std::vector<EngravingItem*> children = collectChildren(chord);

        for (EngravingItem* child : children) {
            if (child->isStem() || child->isBeam()) {
                EXPECT_TRUE(child->visible());
                continue;
            }

            // [THEN] Items from other chords are still hidden
            if (chord != firstChord) {
                EXPECT_FALSE(child->visible());
                continue;
            }

            if (child == firstChordNote) {
                continue;
            }

            // [THEN] Notes of the first chord are hidden (except for the first note)
            //        All other parts of the chord are visible
            if (child->isNote()) {
                EXPECT_FALSE(child->visible());
            } else {
                EXPECT_TRUE(child->visible());
            }
        }
    }

    delete score;
}

TEST_F(Engraving_ChangeVisibilityTests, CmdToggleVisible)
{
    MasterScore* score = ScoreRW::readScore(CHANGEVISIBILITY_DATA_DIR + u"changevisibility.mscx");
    ASSERT_TRUE(score);

    // [GIVEN] First measure
    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    // [GIVEN] Items on the first measure
    EngravingItemList items = measure->childrenItems();
    ASSERT_FALSE(items.empty());

    // [WHEN] Select the first measure and call cmdToggleVisible()
    score->select(measure);

    score->startCmd();
    score->cmdToggleVisible();
    score->endCmd();

    // [THEN] Everything on the first measure is hidden
    // (excluding these items)
    std::unordered_set<ElementType> alwaysVisibleItems {
        ElementType::CLEF,
        ElementType::KEYSIG,
        ElementType::STAFF_LINES,
        ElementType::SEGMENT,
    };

    for (const EngravingItem* item : items) {
        if (mu::contains(alwaysVisibleItems, item->type())) {
            EXPECT_TRUE(item->visible());
        } else {
            EXPECT_FALSE(item->visible());
        }
    }

    // [WHEN] Call cmdToggleVisible() again
    score->startCmd();
    score->cmdToggleVisible();
    score->endCmd();

    // [THEN] Everything on the first measure is visible
    for (const EngravingItem* item : items) {
        EXPECT_TRUE(item->visible());
    }

    // [GIVEN] First chord
    Chord* chord = findChord(score, 0);
    ASSERT_TRUE(chord);

    // [WHEN] Select notes in the first chord and call cmdToggleVisible()
    for (Note* note : chord->notes()) {
        score->select(note, SelectType::ADD);
    }

    score->startCmd();
    score->cmdToggleVisible();
    score->endCmd();

    // [THEN] The notes are hidden
    for (Note* note : chord->notes()) {
        EXPECT_FALSE(note->visible());
    }

    // [WHEN] Select the first measure and call cmdToggleVisible() again
    score->select(measure);

    score->startCmd();
    score->cmdToggleVisible();
    score->endCmd();

    // [THEN] Everything on the first measure is visible
    for (const EngravingItem* item : items) {
        EXPECT_TRUE(item->visible());
    }

    // [THEN] Including the previously hidden notes
    for (Note* note : chord->notes()) {
        EXPECT_TRUE(note->visible());
    }
}
