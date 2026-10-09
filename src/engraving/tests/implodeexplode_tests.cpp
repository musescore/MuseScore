/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "engraving/dom/chord.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/tie.h"
#include "utils/testutils.h"

#include "engraving/dom/masterscore.h"
#include "engraving/editing/implodeexplode.h"
#include "engraving/editing/transaction/undostack.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu::engraving;

static const String IMPLODEEXP_DATA_DIR("implode_explode_data/");

class Engraving_ImplodeExplodeTests : public ::testing::Test
{
public:
    void testUndoExplode(String fileName);
    void testUndoImplode(String fileName);
};

void Engraving_ImplodeExplodeTests::testUndoExplode(String fileName)
{
    String readFile(IMPLODEEXP_DATA_DIR + fileName + ".mscx");
    String writeFile1(fileName + "01-test.mscx");
    String reference1(IMPLODEEXP_DATA_DIR + fileName + "01-ref.mscx");
    String writeFile2(fileName + "02-test.mscx");
    String reference2(IMPLODEEXP_DATA_DIR + fileName + "02-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    EXPECT_TRUE(score);
    score->doLayout();

    // select all
    score->startCmd(TranslatableString::untranslatable("Implode/explode select all"));
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd(TranslatableString::untranslatable("Implode/explode tests"));
    ImplodeExplode::explode(score);
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    delete score;
}

void Engraving_ImplodeExplodeTests::testUndoImplode(String filename)
{
    String readFile(IMPLODEEXP_DATA_DIR + filename + ".mscx");
    String writeFile1(filename + "01-test.mscx");
    String reference1(IMPLODEEXP_DATA_DIR + filename + "01-ref.mscx");
    String writeFile2(filename + "02-test.mscx");
    String reference2(IMPLODEEXP_DATA_DIR + filename + "02-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    EXPECT_TRUE(score);
    score->doLayout();

    // select all
    score->startCmd(TranslatableString::untranslatable("Implode/explode select all"));
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd(TranslatableString::untranslatable("Implode/explode tests"));
    ImplodeExplode::implode(score);
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    delete score;
}

TEST_F(Engraving_ImplodeExplodeTests, undoExplode)
{
    testUndoExplode(u"undoExplode");
}

TEST_F(Engraving_ImplodeExplodeTests, undoImplode)
{
    testUndoImplode(u"undoImplode");
}

TEST_F(Engraving_ImplodeExplodeTests, undoImplodeVoice)
{
    testUndoImplode(u"undoImplodeVoice");
}

TEST_F(Engraving_ImplodeExplodeTests, implodeScore)
{
    testUndoImplode(u"implodeScore");
}

TEST_F(Engraving_ImplodeExplodeTests, explodeDynamics)
{
    testUndoExplode(u"explodeDynamics");
}

TEST_F(Engraving_ImplodeExplodeTests, implodeDynamics)
{
    testUndoImplode(u"implodeDynamics");
}

TEST_F(Engraving_ImplodeExplodeTests, implodeArticulations)
{
    testUndoImplode(u"implodeArticulations");
}

TEST_F(Engraving_ImplodeExplodeTests, explodeLinkedTies)
{
    for (bool fromPart : { false, true }) {
        SCOPED_TRACE(fromPart);
        MasterScore* score = ScoreRW::readScore(IMPLODEEXP_DATA_DIR + u"explode-linked-ties.mscx");
        ASSERT_TRUE(score);
        Score* part = TestUtils::createPart(score);
        ASSERT_TRUE(part);
        auto check = [&](bool exploded) {
            for (Score* s : { static_cast<Score*>(score), part }) {
                for (track_idx_t track : { 0, exploded ? 4 : 1 }) {
                    Segment* start = s->tick2segment(Fraction(0, 1));
                    Segment* end = s->tick2segment(Fraction(1, 2));
                    ASSERT_TRUE(start && end);
                    ASSERT_TRUE(start->element(track) && start->element(track)->isChord());
                    ASSERT_TRUE(end->element(track) && end->element(track)->isChord());
                    Note* startNote = toChord(start->element(track))->upNote();
                    Note* endNote = toChord(end->element(track))->upNote();
                    ASSERT_TRUE(startNote->tieFor());
                    EXPECT_EQ(startNote->tieFor()->startNote(), startNote);
                    EXPECT_EQ(startNote->tieFor()->endNote(), endNote);
                    EXPECT_EQ(endNote->tieBack(), startNote->tieFor());
                    EXPECT_EQ(startNote->tieFor()->score(), s);
                    Segment* nextMeasure = s->tick2segment(Fraction(1, 1));
                    ASSERT_TRUE(nextMeasure && nextMeasure->element(track) && nextMeasure->element(track)->isChord());
                    Note* nextNote = toChord(nextMeasure->element(track))->upNote();
                    ASSERT_TRUE(endNote->tieFor());
                    EXPECT_EQ(endNote->tieFor()->endNote(), nextNote);
                    EXPECT_EQ(nextNote->tieBack(), endNote->tieFor());
                }
            }
        };
        check(false);
        Score* owner = fromPart ? part : score;
        owner->cmdSelectAll();
        score->startCmd(TranslatableString::untranslatable("Explode linked ties"));
        ASSERT_TRUE(ImplodeExplode::explode(owner));
        score->endCmd();
        check(true);
        score->undoRedo(true, nullptr);
        check(false);
        score->undoRedo(false, nullptr);
        check(true);
        ASSERT_FALSE(HasFailure());
        ASSERT_TRUE(ScoreRW::saveScore(score, u"explode-linked-ties-test.mscx"));
        delete score;
        score = ScoreRW::readScore(u"explode-linked-ties-test.mscx", true);
        ASSERT_TRUE(score);
        ASSERT_EQ(score->excerpts().size(), 1u);
        part = score->excerpts().front()->excerptScore();
        check(true);
        delete score;
    }
}

TEST_F(Engraving_ImplodeExplodeTests, explodeLinkedTiesAtRangeEnd)
{
    MasterScore* score = ScoreRW::readScore(IMPLODEEXP_DATA_DIR + u"explode-linked-ties.mscx");
    ASSERT_TRUE(score);
    Score* part = TestUtils::createPart(score);
    ASSERT_TRUE(part);
    score->select(score->firstMeasure(), SelectType::SINGLE, 0);
    score->select(score->firstMeasure(), SelectType::RANGE, 1);
    score->startCmd(TranslatableString::untranslatable("Explode one measure"));
    ASSERT_TRUE(ImplodeExplode::explode(score));
    score->endCmd();
    auto check = [&]() {
        for (Score* s : { static_cast<Score*>(score), part }) {
            Note* start = toChord(s->tick2segment(Fraction(0, 1))->element(4))->upNote();
            Note* end = toChord(s->tick2segment(Fraction(1, 2))->element(4))->upNote();
            ASSERT_TRUE(start->tieFor());
            EXPECT_EQ(start->tieFor()->endNote(), end);
            EXPECT_EQ(end->tieBack(), start->tieFor());
            EXPECT_FALSE(end->tieFor());
        }
    };
    check();
    score->undoRedo(true, nullptr);
    for (Score* s : { static_cast<Score*>(score), part }) {
        Note* start = toChord(s->tick2segment(Fraction(1, 2))->element(1))->upNote();
        Note* end = toChord(s->tick2segment(Fraction(1, 1))->element(1))->upNote();
        ASSERT_TRUE(start->tieFor());
        EXPECT_EQ(start->tieFor()->endNote(), end);
        EXPECT_EQ(end->tieBack(), start->tieFor());
    }
    score->undoRedo(false, nullptr);
    check();
    delete score;
}
