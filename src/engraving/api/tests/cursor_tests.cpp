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

#include "engraving/api/v1/apistructs.h"
#include "engraving/api/v1/cursor.h"

#include "engraving/dom/chordrest.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/tuplet.h"

#include "engraving/tests/utils/scorerw.h"

using namespace muse;
using namespace mu::engraving;

// these tests create tuplets whose total duration isn't a single note value (#35191);
// the engine tests for Score::addTuplet() cover the other cases

static const String CURSOR_DATA_DIR("cursor_data/");

static void rewindTo(apiv1::Cursor& cursor, int elementIndex)
{
    cursor.rewind(apiv1::Cursor::SCORE_START);
    for (int i = 0; i < elementIndex; ++i) {
        cursor.next();
    }
}

// empty.mscx with eight eighths in m1
static MasterScore* readScoreWithEighths()
{
    MasterScore* score = ScoreRW::readScore(CURSOR_DATA_DIR + u"empty.mscx");
    if (!score) {
        return nullptr;
    }

    apiv1::Cursor cursor(score);
    cursor.rewind(apiv1::Cursor::SCORE_START);
    cursor.setDuration(1, 8);
    score->startCmd(TranslatableString::untranslatable("Engraving API cursor tests"));
    for (int i = 0; i < 8; ++i) {
        cursor.addNote(60);
    }
    score->endCmd();

    return score;
}

static void addTuplet(Score* score, apiv1::Cursor& cursor, const Fraction& ratio, const Fraction& duration)
{
    apiv1::Fraction apiRatio(ratio);
    apiv1::Fraction apiDuration(duration);

    score->startCmd(TranslatableString::untranslatable("Engraving API cursor tests"));
    cursor.addTuplet(&apiRatio, &apiDuration);
    score->endCmd();
}

static ChordRest* chordRestAt(Score* score, const Fraction& tick)
{
    Segment* segment = score->tick2segment(tick, true, SegmentType::ChordRest);
    return segment ? toChordRest(segment->element(0)) : nullptr;
}

static void expectTuplet(Score* score, const Fraction& tick, const Fraction& ratio, const Fraction& ticks)
{
    ChordRest* cr = chordRestAt(score, tick);
    ASSERT_TRUE(cr);
    Tuplet* tuplet = cr->tuplet();
    ASSERT_TRUE(tuplet);
    EXPECT_EQ(tuplet->tick(), tick);
    EXPECT_EQ(tuplet->ratio(), ratio);
    EXPECT_EQ(tuplet->ticks(), ticks);
    EXPECT_EQ(tuplet->elements().size(), static_cast<size_t>(ratio.numerator()));
}

// 8 eighths in the time of 5 on the first eighth of m1: the tuplet is longer than the chord
TEST(Engraving_ApiCursorTests, addTupletLongerThanChordRest)
{
    MasterScore* score = readScoreWithEighths();
    ASSERT_TRUE(score);

    apiv1::Cursor cursor(score);
    rewindTo(cursor, 0);
    addTuplet(score, cursor, Fraction(8, 5), Fraction(5, 8));

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();
    expectTuplet(score, Fraction(0, 1), Fraction(8, 5), Fraction(5, 8));

    // the last three eighths of m1 are kept
    ChordRest* next = chordRestAt(score, Fraction(5, 8));
    ASSERT_TRUE(next);
    EXPECT_TRUE(next->isChord());
    EXPECT_FALSE(next->tuplet());

    // the score is also valid after saving and reopening it
    ASSERT_TRUE(ScoreRW::saveScore(score, u"cursor-addTuplet-longer.mscx"));
    MasterScore* reloaded = ScoreRW::readScore(u"cursor-addTuplet-longer.mscx", true);
    ASSERT_TRUE(reloaded);
    ret = reloaded->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();
    expectTuplet(reloaded, Fraction(0, 1), Fraction(8, 5), Fraction(5, 8));

    delete reloaded;
    delete score;
}

// 8 eighths in the time of 5 on the measure rest of m1
TEST(Engraving_ApiCursorTests, addTupletOnMeasureRest)
{
    MasterScore* score = ScoreRW::readScore(CURSOR_DATA_DIR + u"empty.mscx");
    ASSERT_TRUE(score);

    apiv1::Cursor cursor(score);
    rewindTo(cursor, 0);
    addTuplet(score, cursor, Fraction(8, 5), Fraction(5, 8));

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();
    expectTuplet(score, Fraction(0, 1), Fraction(8, 5), Fraction(5, 8));

    // the rest of the measure is filled with rests
    ChordRest* next = chordRestAt(score, Fraction(5, 8));
    ASSERT_TRUE(next);
    EXPECT_TRUE(next->isRest());
    EXPECT_FALSE(next->tuplet());

    delete score;
}

// a triplet whose total is a single note value works as before
TEST(Engraving_ApiCursorTests, addTupletSingleNoteValue)
{
    MasterScore* score = readScoreWithEighths();
    ASSERT_TRUE(score);

    apiv1::Cursor cursor(score);
    rewindTo(cursor, 0);
    addTuplet(score, cursor, Fraction(3, 2), Fraction(1, 4));

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();
    expectTuplet(score, Fraction(0, 1), Fraction(3, 2), Fraction(1, 4));

    delete score;
}

// notes added after addTuplet() fill the tuplet, as plugins do
TEST(Engraving_ApiCursorTests, addTupletThenAddNotes)
{
    MasterScore* score = readScoreWithEighths();
    ASSERT_TRUE(score);

    apiv1::Cursor cursor(score);
    rewindTo(cursor, 0);

    apiv1::Fraction ratio(Fraction(8, 5));
    apiv1::Fraction duration(Fraction(5, 8));
    score->startCmd(TranslatableString::untranslatable("Engraving API cursor tests"));
    cursor.addTuplet(&ratio, &duration);
    for (int i = 0; i < 8; ++i) {
        cursor.addNote(60 + i);
    }
    score->endCmd();

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();
    expectTuplet(score, Fraction(0, 1), Fraction(8, 5), Fraction(5, 8));

    Tuplet* tuplet = chordRestAt(score, Fraction(0, 1))->tuplet();
    ASSERT_TRUE(tuplet);
    for (DurationElement* element : tuplet->elements()) {
        EXPECT_TRUE(element->isChord());
    }

    delete score;
}

// a tuplet that would cross the end of the tuplet it starts in is refused
TEST(Engraving_ApiCursorTests, addTupletCrossingParentTuplet)
{
    MasterScore* score = readScoreWithEighths();
    ASSERT_TRUE(score);

    // a quintuplet over the first eighth of m1
    apiv1::Cursor cursor(score);
    rewindTo(cursor, 0);
    addTuplet(score, cursor, Fraction(5, 4), Fraction(1, 8));
    Tuplet* quintuplet = chordRestAt(score, Fraction(0, 1))->tuplet();
    ASSERT_TRUE(quintuplet);

    // 3/8 from its second note would end after the quintuplet
    rewindTo(cursor, 1);
    addTuplet(score, cursor, Fraction(2, 3), Fraction(3, 8));

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();
    ChordRest* second = chordRestAt(score, Fraction(1, 40));
    ASSERT_TRUE(second);
    EXPECT_EQ(second->tuplet(), quintuplet);
    EXPECT_EQ(second->ticks(), Fraction(1, 32));

    delete score;
}
