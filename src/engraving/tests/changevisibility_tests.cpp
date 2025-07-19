/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "dom/accidental.h"
#include "dom/measure.h"
#include "dom/chord.h"
#include "dom/note.h"
#include "dom/ornament.h"
#include "dom/rest.h"
#include "dom/stem.h"
#include "dom/hook.h"
#include "dom/beam.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

static const String CHANGEVISIBILITY_DATA_DIR(u"changevisibility_data/");

class Engraving_ChangeVisibilityTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_score = ScoreRW::readScore(CHANGEVISIBILITY_DATA_DIR + u"changevisibility.mscx");
    }

    void TearDown() override
    {
        delete m_score;
        m_score = nullptr;
    }

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

    Chord* findChord(int tick) const
    {
        IF_ASSERT_FAILED(m_score) {
            return nullptr;
        }

        ChordRest* cr = m_score->findCR(Fraction::fromTicks(tick), 0);
        if (cr->isChord()) {
            return toChord(cr);
        }

        return nullptr;
    }

    Rest* findRest(int tick) const
    {
        IF_ASSERT_FAILED(m_score) {
            return nullptr;
        }

        ChordRest* cr = m_score->findCR(Fraction::fromTicks(tick), 0);
        if (cr->isRest()) {
            return toRest(cr);
        }

        return nullptr;
    }

    MasterScore* m_score = nullptr;
};

TEST_F(Engraving_ChangeVisibilityTests, UndoChangeVisible_SingleNoteChord)
{
    ASSERT_TRUE(m_score);

    // [GIVEN] Chord containing only one note
    Chord* chord = findChord(0);
    ASSERT_TRUE(chord);

    ASSERT_TRUE(chord->notes().size() == 1);
    Note* note = chord->notes().front();
    ASSERT_TRUE(note);

    // [GIVEN] All items attached to this chord
    std::vector<EngravingItem*> children = collectChildren(chord);
    ASSERT_FALSE(children.empty());

    // [WHEN] Hide the note
    m_score->undoChangeVisible(note, false);

    // [THEN] Everything in the chord is hidden
    for (EngravingItem* child : children) {
        EXPECT_FALSE(child->visible());
    }

    // [WHEN] Show the note
    m_score->undoChangeVisible(note, true);

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
    m_score->undoChangeVisible(dot, false);
    m_score->undoChangeVisible(stem, false);

    // [THEN] Everything in the chord is visible, except the parts that were hidden manually
    for (EngravingItem* child : children) {
        if (child == dot || child == stem) {
            EXPECT_FALSE(child->visible());
        } else {
            EXPECT_TRUE(child->visible());
        }
    }
}

TEST_F(Engraving_ChangeVisibilityTests, UndoChangeVisible_RestWithDot)
{
    ASSERT_TRUE(m_score);

    // [GIVEN] Rest with a dot
    Rest* rest = findRest(480);
    ASSERT_TRUE(rest);

    ASSERT_TRUE(!rest->dotList().empty());
    NoteDot* dot = rest->dotList().front();

    // [WHEN] Hide the rest
    m_score->undoChangeVisible(rest, false);

    // [THEN] Rest and its dot are hidden
    EXPECT_FALSE(rest->visible());
    EXPECT_FALSE(dot->visible());

    // [WHEN] Show the rest
    m_score->undoChangeVisible(rest, true);

    // [THEN] Rest and its dot are visible
    EXPECT_TRUE(rest->visible());
    EXPECT_TRUE(dot->visible());

    // [WHEN] We can also hide the dot
    m_score->undoChangeVisible(dot, false);

    // [THEN] Rest is visible, but the dot is not
    EXPECT_TRUE(rest->visible());
    EXPECT_FALSE(dot->visible());
}

TEST_F(Engraving_ChangeVisibilityTests, UndoChangeVisible_IgnoredElements)
{
    ASSERT_TRUE(m_score);

    // [GIVEN] Note with attached grace notes, lyrics and slur
    Chord* chord = findChord(1200);
    ASSERT_TRUE(chord);

    const std::vector<Chord*>& graceNotes = chord->graceNotes();
    ASSERT_TRUE(!graceNotes.empty());

    ASSERT_TRUE(chord->notes().size() == 1);
    Note* note = chord->notes().front();
    ASSERT_TRUE(note);

    // [WHEN] Hide the note
    m_score->undoChangeVisible(note, false);

    // [THEN] The note is hidden, but the grace notes, lyrics and slur are still visible
    EXPECT_FALSE(note->visible());

    const std::unordered_set<ElementType> IGNORED_TYPES {
        ElementType::CHORD,
        ElementType::SLUR,
        ElementType::LYRICS,
    };

    for (EngravingObject* child : chord->scanChildren()) {
        if (muse::contains(IGNORED_TYPES, child->type())) {
            EngravingItem* item = toEngravingItem(child);
            EXPECT_TRUE(item->visible());
        }
    }

    for (const Chord* graceNote : graceNotes) {
        EXPECT_TRUE(graceNote->visible());
    }

    // [WHEN] Show the note
    m_score->undoChangeVisible(note, true);

    // [THEN] Everything is visible
    EXPECT_TRUE(note->visible());

    for (EngravingObject* child : chord->scanChildren()) {
        if (muse::contains(IGNORED_TYPES, child->type())) {
            EngravingItem* item = toEngravingItem(child);
            EXPECT_TRUE(item->visible());
        }
    }

    for (const Chord* graceNote : graceNotes) {
        EXPECT_TRUE(graceNote->visible());
    }

    // [WHEN] We can hide any grace note
    m_score->undoChangeVisible(graceNotes[0], false);

    // [THEN] Everything is visible except the previously hidden grace note
    EXPECT_TRUE(note->visible());

    for (size_t i = 0; i < graceNotes.size(); ++i) {
        if (i == 0) {
            EXPECT_FALSE(graceNotes[i]->visible());
        } else {
            EXPECT_TRUE(graceNotes[i]->visible());
        }
    }
}

TEST_F(Engraving_ChangeVisibilityTests, UndoChangeVisible_ChordContainingSeveralNotes)
{
    ASSERT_TRUE(m_score);

    // [GIVEN] Chord containing several notes
    Chord* chord = findChord(1920);
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
    m_score->undoChangeVisible(firstNote, false);

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
    m_score->undoChangeVisible(firstNote, false);

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
        m_score->undoChangeVisible(note, false);
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
}

TEST_F(Engraving_ChangeVisibilityTests, UndoChangeVisible_ChordsConnectedWithBeam)
{
    ASSERT_TRUE(m_score);

    // [GIVEN] First chord under the beam
    Chord* firstChord = findChord(3840);
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
        m_score->undoChangeVisible(note, false);
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
                m_score->undoChangeVisible(child, false);
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
    m_score->undoChangeVisible(firstChordNote, true);

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
}

TEST_F(Engraving_ChangeVisibilityTests, UndoChangeVisible_Ornaments)
{
    ASSERT_TRUE(m_score);

    Measure* measure = m_score->tick2measure(Fraction(3, 1));
    ASSERT_TRUE(measure);

    Chord* chord = toChord(measure->first()->elementAt(0));
    ASSERT_TRUE(chord);
    Note* note = chord->upNote();
    ASSERT_TRUE(note);
    Ornament* ornament = toOrnament(chord->articulations().front());
    ASSERT_TRUE(ornament);
    Accidental* accidental = ornament->accidentalAbove();
    ASSERT_TRUE(accidental);

    // NOTE invisible makes ORNAMENT invisible
    m_score->undoChangeVisible(note, false);
    ASSERT_FALSE(ornament->visible());

    // ORNAMENT visible, but note stays invisible
    m_score->undoChangeVisible(ornament, true);
    ASSERT_TRUE(ornament->visible());
    ASSERT_FALSE(note->visible());

    // Ornament accidental invisible, but ornament stays visible
    m_score->undoChangeVisible(accidental, false);
    ASSERT_FALSE(accidental->visible());
    ASSERT_TRUE(ornament->visible());

    chord = toChord(chord->segment()->next()->elementAt(0));
    ASSERT_TRUE(chord);
    note = chord->upNote();
    ASSERT_TRUE(note);
    ornament = toOrnament(chord->articulations().front());
    ASSERT_TRUE(ornament);
    Chord* cueNoteChord = ornament->cueNoteChord();
    ASSERT_TRUE(cueNoteChord);
    Note* cueNote = cueNoteChord->upNote();
    ASSERT_TRUE(cueNote);

    // ORNAMENT invisible, cue note also becomes invisible
    m_score->undoChangeVisible(ornament, false);
    ASSERT_FALSE(ornament->visible());
    ASSERT_FALSE(cueNote->visible());
    for (EngravingItem* el : cueNote->el()) {
        ASSERT_FALSE(el->visible());
    }

    // CUE NOTE visible, but ornament stays invisible
    m_score->undoChangeVisible(cueNote, true);
    ASSERT_FALSE(ornament->visible());
    ASSERT_TRUE(cueNote->visible());
    for (EngravingItem* el : cueNote->el()) {
        ASSERT_TRUE(el->visible());
    }
}

TEST_F(Engraving_ChangeVisibilityTests, CmdToggleVisible)
{
    ASSERT_TRUE(m_score);

    // [GIVEN] First measure
    Measure* measure = m_score->firstMeasure();
    ASSERT_TRUE(measure);

    // [GIVEN] Items on the first measure
    EngravingItemList items = measure->childrenItems();
    ASSERT_FALSE(items.empty());

    // [WHEN] Select the first measure and call cmdToggleVisible()
    m_score->select(measure);

    m_score->startCmd(TranslatableString::untranslatable("Change visibility tests"));
    m_score->cmdToggleVisible();
    m_score->endCmd();

    // [THEN] Everything on the first measure is hidden
    // (excluding these items)
    std::unordered_set<ElementType> alwaysVisibleItems {
        ElementType::CLEF,
        ElementType::KEYSIG,
        ElementType::STAFF_LINES,
        ElementType::SEGMENT,
    };

    for (const EngravingItem* item : items) {
        if (muse::contains(alwaysVisibleItems, item->type())) {
            EXPECT_TRUE(item->visible());
        } else {
            EXPECT_FALSE(item->visible());
        }
    }

    // [WHEN] Call cmdToggleVisible() again
    m_score->startCmd(TranslatableString::untranslatable("Change visibility tests"));
    m_score->cmdToggleVisible();
    m_score->endCmd();

    // [THEN] Everything on the first measure is visible
    for (const EngravingItem* item : items) {
        EXPECT_TRUE(item->visible());
    }

    // [GIVEN] First chord
    Chord* chord = findChord(0);
    ASSERT_TRUE(chord);

    // [WHEN] Select notes in the first chord and call cmdToggleVisible()
    for (Note* note : chord->notes()) {
        m_score->select(note, SelectType::ADD);
    }

    m_score->startCmd(TranslatableString::untranslatable("Change visibility tests"));
    m_score->cmdToggleVisible();
    m_score->endCmd();

    // [THEN] The notes are hidden
    for (Note* note : chord->notes()) {
        EXPECT_FALSE(note->visible());
    }

    // [WHEN] Select the first measure and call cmdToggleVisible() again
    m_score->select(measure);

    m_score->startCmd(TranslatableString::untranslatable("Change visibility tests"));
    m_score->cmdToggleVisible();
    m_score->endCmd();

    // [THEN] Everything on the first measure is visible
    for (const EngravingItem* item : items) {
        EXPECT_TRUE(item->visible());
    }

    // [THEN] Including the previously hidden notes
    for (Note* note : chord->notes()) {
        EXPECT_TRUE(note->visible());
    }
}
