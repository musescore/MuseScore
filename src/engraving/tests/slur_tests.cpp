/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <gtest/gtest.h>

#include "engraving/dom/barline.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/hammeronpulloff.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/editing/editslur.h"
#include "engraving/editing/edittie.h"
#include "engraving/editing/noteinput.h"
#include "engraving/editing/transaction/transaction.h"
#include "engraving/editing/transaction/undostack.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

static const String SLUR_DATA_DIR(u"slur_data/");

class Engraving_SlurTests : public ::testing::Test
{
};

// addSlurs.mscx: m1 eight eighths, m2 half note + half rest, m3 a 9:8 eighth tuplet, m4 eight eighths

static ChordRest* chordRestAt(Measure* measure, size_t index)
{
    size_t i = 0;
    for (Segment* s = measure->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        if (i++ == index) {
            return toChordRest(s->element(0));
        }
    }
    return nullptr;
}

static Note* noteAt(Measure* measure, size_t index)
{
    return toChord(chordRestAt(measure, index))->upNote();
}

static std::vector<Slur*> slurs(Score* score)
{
    std::vector<Slur*> result;
    for (const auto& it : score->spanner()) {
        if (it.second->isSlur()) {
            result.push_back(toSlur(it.second));
        }
    }
    return result;
}

// Adds slurs to the selection, as the slur commands do
static std::vector<EditSlur::AddedSlur> addSlurs(Score* score, const Slur* slurTemplate = nullptr)
{
    std::vector<EditSlur::AddedSlur> added;
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving slur tests"), [&](Transaction& tx) {
        added = EditSlur::addSlurs(tx, score, slurTemplate);
    });
    return added;
}

// Adds four quarters in voice 2 of a measure; returns them
static std::vector<Chord*> addVoice2Quarters(MasterScore* score, Measure* measure)
{
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving slur tests"), [&](Transaction&) {
        for (int i = 0; i < 4; ++i) {
            Segment* s = measure->undoGetSegment(SegmentType::ChordRest, measure->tick() + Fraction(i, 4));
            score->setNoteRest(s, 1, NoteVal(55 + i), Fraction(1, 4));
        }
    });
    std::vector<Chord*> chords;
    for (int i = 0; i < 4; ++i) {
        Segment* s = measure->findSegment(SegmentType::ChordRest, measure->tick() + Fraction(i, 4));
        EngravingItem* e = s ? s->element(1) : nullptr;
        chords.push_back(e && e->isChord() ? toChord(e) : nullptr);
    }
    return chords;
}

TEST_F(Engraving_SlurTests, addSlurFromNote)
{
    MasterScore* score = ScoreRW::readScore(SLUR_DATA_DIR + u"addSlurs.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = score->firstMeasure();

    // a slur from a single note goes to the next chord
    score->select(noteAt(m1, 2), SelectType::SINGLE);
    const std::vector<EditSlur::AddedSlur> added = addSlurs(score);
    ASSERT_EQ(added.size(), 1);
    EXPECT_FALSE(added[0].replacedSlur);
    EXPECT_TRUE(added[0].toNextChord);
    Slur* slur = added[0].slur;
    ASSERT_TRUE(slur);
    EXPECT_EQ(slur->startElement(), chordRestAt(m1, 2));
    EXPECT_EQ(slur->endElement(), chordRestAt(m1, 3));
    EXPECT_EQ(slurs(score).size(), 1);
    EXPECT_TRUE(score->sanityCheck());

    // the same command again gives the slur already there, with no undo step
    const std::vector<EditSlur::AddedSlur> again = addSlurs(score);
    ASSERT_EQ(again.size(), 1);
    EXPECT_EQ(again[0].slur, slur);
    EXPECT_FALSE(again[0].replacedSlur);
    EXPECT_EQ(slurs(score).size(), 1);

    score->undoRedo(true, nullptr);
    EXPECT_TRUE(slurs(score).empty());
    EXPECT_FALSE(score->undoStack()->canUndo());

    delete score;
}

TEST_F(Engraving_SlurTests, addSlursToRange)
{
    MasterScore* score = ScoreRW::readScore(SLUR_DATA_DIR + u"addSlurs.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = score->firstMeasure();

    // one slur from the first to the last chord of the range
    score->select(chordRestAt(m1, 1), SelectType::RANGE);
    score->select(chordRestAt(m1, 4), SelectType::RANGE);
    const std::vector<EditSlur::AddedSlur> added = addSlurs(score);
    ASSERT_EQ(added.size(), 1);
    EXPECT_EQ(added[0].slur->startElement(), chordRestAt(m1, 1));
    EXPECT_EQ(added[0].slur->endElement(), chordRestAt(m1, 4));
    EXPECT_TRUE(score->sanityCheck());

    // the range is kept
    EXPECT_TRUE(score->selection().isRange());

    score->undoRedo(true, nullptr);
    EXPECT_TRUE(slurs(score).empty());

    delete score;
}

TEST_F(Engraving_SlurTests, addSlurFromNoteInVoice2)
{
    MasterScore* score = ScoreRW::readScore(SLUR_DATA_DIR + u"addSlurs.mscx");
    ASSERT_TRUE(score);
    Measure* m4 = score->crMeasure(3);
    const std::vector<Chord*> voice2 = addVoice2Quarters(score, m4);
    ASSERT_TRUE(voice2[1] && voice2[2]);

    // the slur goes to the next chord in the same voice
    score->select(voice2[1]->upNote(), SelectType::SINGLE);
    const std::vector<EditSlur::AddedSlur> added = addSlurs(score);
    ASSERT_EQ(added.size(), 1);
    EXPECT_TRUE(added[0].toNextChord);
    EXPECT_EQ(added[0].slur->startElement(), voice2[1]);
    EXPECT_EQ(added[0].slur->endElement(), voice2[2]);
    EXPECT_EQ(added[0].slur->track(), 1);
    EXPECT_TRUE(score->sanityCheck());

    delete score;
}

TEST_F(Engraving_SlurTests, addSlursToRangeInTwoVoices)
{
    MasterScore* score = ScoreRW::readScore(SLUR_DATA_DIR + u"addSlurs.mscx");
    ASSERT_TRUE(score);
    Measure* m4 = score->crMeasure(3);
    const std::vector<Chord*> voice2 = addVoice2Quarters(score, m4);
    ASSERT_TRUE(voice2[0] && voice2[3]);

    // one slur per voice, each from its first to its last chord
    score->deselectAll();
    score->select(chordRestAt(m4, 0), SelectType::RANGE);
    score->select(chordRestAt(m4, 7), SelectType::RANGE);
    const std::vector<EditSlur::AddedSlur> added = addSlurs(score);
    ASSERT_EQ(added.size(), 2);
    EXPECT_EQ(added[0].slur->startElement(), chordRestAt(m4, 0));
    EXPECT_EQ(added[0].slur->endElement(), chordRestAt(m4, 7));
    EXPECT_EQ(added[1].slur->startElement(), voice2[0]);
    EXPECT_EQ(added[1].slur->endElement(), voice2[3]);
    EXPECT_TRUE(score->sanityCheck());

    score->undoRedo(true, nullptr);
    EXPECT_TRUE(slurs(score).empty());

    delete score;
}

TEST_F(Engraving_SlurTests, addSlurToList)
{
    MasterScore* score = ScoreRW::readScore(SLUR_DATA_DIR + u"addSlurs.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = score->firstMeasure();

    // one slur from the first to the last selected chord, whatever the selection order
    score->select(noteAt(m1, 6), SelectType::SINGLE);
    score->select(noteAt(m1, 3), SelectType::ADD);
    ASSERT_TRUE(score->selection().isList());
    const std::vector<EditSlur::AddedSlur> added = addSlurs(score);
    ASSERT_EQ(added.size(), 1);
    EXPECT_EQ(added[0].slur->startElement(), chordRestAt(m1, 3));
    EXPECT_EQ(added[0].slur->endElement(), chordRestAt(m1, 6));
    EXPECT_FALSE(added[0].slur->isOutgoing());
    EXPECT_FALSE(added[0].toNextChord);
    EXPECT_TRUE(score->sanityCheck());

    score->undoRedo(true, nullptr);
    EXPECT_TRUE(slurs(score).empty());

    delete score;
}

TEST_F(Engraving_SlurTests, addSlurToListOnOneChord)
{
    MasterScore* score = ScoreRW::readScore(SLUR_DATA_DIR + u"addSlurs.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = score->firstMeasure();
    Chord* chord = toChord(chordRestAt(m1, 2));
    Note* lowerNote = chord->upNote();
    Note* upperNote = nullptr;
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving slur tests"), [&](Transaction& tx) {
        upperNote = NoteInput::addNote(tx, score, chord, NoteVal(lowerNote->pitch() + 4));
    });
    ASSERT_TRUE(upperNote);

    // two notes of one chord: a slur to the next chord, as from a single note
    score->select(lowerNote, SelectType::SINGLE);
    score->select(upperNote, SelectType::ADD);
    ASSERT_TRUE(score->selection().isList());
    const std::vector<EditSlur::AddedSlur> added = addSlurs(score);
    ASSERT_EQ(added.size(), 1);
    EXPECT_TRUE(added[0].toNextChord);
    EXPECT_EQ(added[0].slur->startElement(), chord);
    EXPECT_EQ(added[0].slur->endElement(), chordRestAt(m1, 3));
    EXPECT_EQ(slurs(score).size(), 1);
    EXPECT_TRUE(score->sanityCheck());

    score->undoRedo(true, nullptr);
    EXPECT_TRUE(slurs(score).empty());

    delete score;
}

// Ties every note of a chord to the next chord, as + does outside note input
static void tieToNext(MasterScore* score, Chord* chord)
{
    score->deselectAll();
    for (Note* note : chord->notes()) {
        score->select(note, SelectType::ADD);
    }
    // the tied note takes the input duration, which the app sets from the selection
    score->inputState().update(score->selection());
    EditTie::cmdAddTie(score);
}

TEST_F(Engraving_SlurTests, addSlurBeforeTiedNotes)
{
    MasterScore* score = ScoreRW::readScore(SLUR_DATA_DIR + u"addSlurs.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = score->firstMeasure();
    tieToNext(score, toChord(chordRestAt(m1, 2)));
    tieToNext(score, toChord(chordRestAt(m1, 3)));
    ASSERT_TRUE(toChord(chordRestAt(m1, 2))->allNotesTiedToNext());
    ASSERT_TRUE(toChord(chordRestAt(m1, 3))->allNotesTiedToNext());

    // a slur from the note before tied notes goes to the last of them
    score->select(noteAt(m1, 1), SelectType::SINGLE);
    const std::vector<EditSlur::AddedSlur> added = addSlurs(score);
    ASSERT_EQ(added.size(), 1);
    EXPECT_TRUE(added[0].toNextChord);
    EXPECT_EQ(added[0].slur->startElement(), chordRestAt(m1, 1));
    EXPECT_EQ(added[0].slur->endElement(), chordRestAt(m1, 4));
    EXPECT_TRUE(score->sanityCheck());

    score->undoRedo(true, nullptr);
    EXPECT_TRUE(slurs(score).empty());

    delete score;
}

TEST_F(Engraving_SlurTests, addSlurToListOnTiedChord)
{
    MasterScore* score = ScoreRW::readScore(SLUR_DATA_DIR + u"addSlurs.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = score->firstMeasure();
    Chord* chord = toChord(chordRestAt(m1, 2));
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving slur tests"), [&](Transaction& tx) {
        NoteInput::addNote(tx, score, chord, NoteVal(chord->upNote()->pitch() + 3));
    });
    ASSERT_EQ(chord->notes().size(), 2);
    tieToNext(score, chord);
    tieToNext(score, toChord(chordRestAt(m1, 3)));
    ASSERT_TRUE(chord->allNotesTiedToNext());
    ASSERT_TRUE(toChord(chordRestAt(m1, 3))->allNotesTiedToNext());

    // two notes of a chord tied onwards: a slur past the tied notes, as from a single note
    score->deselectAll();
    score->select(chord->notes()[0], SelectType::ADD);
    score->select(chord->notes()[1], SelectType::ADD);
    ASSERT_TRUE(score->selection().isList());
    const std::vector<EditSlur::AddedSlur> added = addSlurs(score);
    ASSERT_EQ(added.size(), 1);
    EXPECT_TRUE(added[0].toNextChord);
    EXPECT_EQ(added[0].slur->startElement(), chord);
    EXPECT_EQ(added[0].slur->endElement(), chordRestAt(m1, 5));
    EXPECT_TRUE(score->sanityCheck());

    score->undoRedo(true, nullptr);
    EXPECT_TRUE(slurs(score).empty());

    delete score;
}

static BarLine* endBarLine(Measure* measure)
{
    Segment* s = measure->findSegment(SegmentType::EndBarLine, measure->endTick());
    return s ? toBarLine(s->element(0)) : nullptr;
}

// A chord and the barline after it, where a repeat or jump follows: a partial slur to the end of the
// measure, which survives saving and reopening
static void checkOutgoingSlur(MasterScore* score, const String& saveName)
{
    Measure* m1 = score->firstMeasure();
    BarLine* barLine = endBarLine(m1);
    ASSERT_TRUE(barLine);

    score->select(noteAt(m1, 5), SelectType::SINGLE);
    score->select(barLine, SelectType::ADD);
    const std::vector<EditSlur::AddedSlur> added = addSlurs(score);
    ASSERT_EQ(added.size(), 1);
    Slur* slur = added[0].slur;
    EXPECT_TRUE(slur->isOutgoing());
    EXPECT_EQ(slur->startElement(), chordRestAt(m1, 5));
    EXPECT_EQ(slur->endElement(), chordRestAt(m1, 7));
    EXPECT_TRUE(score->sanityCheck());

    ASSERT_TRUE(ScoreRW::saveScore(score, saveName));
    MasterScore* reopened = ScoreRW::readScore(saveName, true);
    ASSERT_TRUE(reopened);
    const std::vector<Slur*> reopenedSlurs = slurs(reopened);
    ASSERT_EQ(reopenedSlurs.size(), 1);
    EXPECT_TRUE(reopenedSlurs.front()->isOutgoing());
    EXPECT_EQ(reopenedSlurs.front()->startElement(), chordRestAt(reopened->firstMeasure(), 5));
    EXPECT_EQ(reopenedSlurs.front()->endElement(), chordRestAt(reopened->firstMeasure(), 7));
    delete reopened;

    score->undoRedo(true, nullptr);
    EXPECT_TRUE(slurs(score).empty());
}

TEST_F(Engraving_SlurTests, addOutgoingSlurToEndRepeat)
{
    MasterScore* score = ScoreRW::readScore(SLUR_DATA_DIR + u"addSlurs.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = score->firstMeasure();
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving slur tests"), [&](Transaction&) {
        score->undoChangeBarLineType(endBarLine(m1), BarLineType::END_REPEAT, true);
    });
    ASSERT_EQ(endBarLine(m1)->barLineType(), BarLineType::END_REPEAT);

    ASSERT_NO_FATAL_FAILURE(checkOutgoingSlur(score, u"addOutgoingSlurToEndRepeat.mscx"));

    delete score;
}

TEST_F(Engraving_SlurTests, addOutgoingSlurBeforeJump)
{
    MasterScore* score = ScoreRW::readScore(SLUR_DATA_DIR + u"addSlurs.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = score->firstMeasure();
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving slur tests"), [&](Transaction&) {
        Jump* jump = Factory::createJump(m1);
        jump->setJumpType(JumpType::DC);
        jump->setTrack(0);
        score->undoAddElement(jump);
    });

    ASSERT_NO_FATAL_FAILURE(checkOutgoingSlur(score, u"addOutgoingSlurBeforeJump.mscx"));

    delete score;
}

TEST_F(Engraving_SlurTests, hammerOnPullOffReplacesSlur)
{
    MasterScore* score = ScoreRW::readScore(SLUR_DATA_DIR + u"addSlurs.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = score->firstMeasure();

    score->select(noteAt(m1, 0), SelectType::SINGLE);
    ASSERT_EQ(addSlurs(score).size(), 1);
    Slur* slur = slurs(score).front();
    ASSERT_TRUE(slur->frontSegment());

    // a hammer-on/pull-off on the selected slur replaces it
    score->select(slur->frontSegment(), SelectType::SINGLE);
    HammerOnPullOff* hopo = Factory::createHammerOnPullOff(score->dummy());
    const std::vector<EditSlur::AddedSlur> added = addSlurs(score, hopo);
    delete hopo;
    ASSERT_EQ(added.size(), 1);
    EXPECT_TRUE(added[0].replacedSlur);
    EXPECT_TRUE(added[0].slur->isHammerOnPullOff());
    EXPECT_EQ(added[0].slur->startElement(), chordRestAt(m1, 0));
    EXPECT_EQ(added[0].slur->endElement(), chordRestAt(m1, 1));
    std::vector<Slur*> scoreSlurs = slurs(score);
    ASSERT_EQ(scoreSlurs.size(), 1);
    EXPECT_EQ(scoreSlurs.front(), added[0].slur);
    EXPECT_TRUE(score->sanityCheck());

    // undo brings the slur back
    score->undoRedo(true, nullptr);
    scoreSlurs = slurs(score);
    ASSERT_EQ(scoreSlurs.size(), 1);
    EXPECT_EQ(scoreSlurs.front(), slur);
    EXPECT_FALSE(slur->isHammerOnPullOff());

    delete score;
}
