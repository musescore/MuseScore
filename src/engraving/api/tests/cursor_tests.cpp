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
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/tuplet.h"

#include "engraving/tests/utils/scorerw.h"

using namespace muse;
using namespace mu::engraving;

// addTuplet.mscx: m1: eight eighths, m2: half note + half rest,
// m3: a 9:8 eighth tuplet, m4: eight eighths
static const String TUPLET_DATA_DIR("../../tests/tuplet_data/");

static void rewindTo(apiv1::Cursor& cursor, int measureIndex, int elementIndex)
{
    cursor.rewind(apiv1::Cursor::SCORE_START);
    for (int i = 0; i < measureIndex; ++i) {
        cursor.nextMeasure();
    }
    for (int i = 0; i < elementIndex; ++i) {
        cursor.next();
    }
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

// the score must also be valid after saving and reopening it
static void expectValidAfterReload(Score* score, const String& saveName)
{
    ASSERT_TRUE(ScoreRW::saveScore(score, saveName));
    MasterScore* reloaded = ScoreRW::readScore(saveName, true);
    ASSERT_TRUE(reloaded);
    muse::Ret ret = reloaded->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();
    delete reloaded;
}

static void expectTuplet(Score* score, const Fraction& tick, const Fraction& ratio, const Fraction& ticks, Tuplet* parent = nullptr)
{
    ChordRest* cr = chordRestAt(score, tick);
    ASSERT_TRUE(cr);
    Tuplet* tuplet = cr->tuplet();
    ASSERT_TRUE(tuplet);
    EXPECT_EQ(tuplet->tick(), tick);
    EXPECT_EQ(tuplet->ratio(), ratio);
    EXPECT_EQ(tuplet->ticks(), ticks);
    EXPECT_EQ(tuplet->elements().size(), static_cast<size_t>(ratio.numerator()));
    EXPECT_EQ(tuplet->tuplet(), parent);
}

// 8 eighths in the time of 5 on the first eighth of m1: the tuplet is longer than the chord
TEST(Engraving_ApiCursorTests, addTupletLongerThanChordRest)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);

    apiv1::Cursor cursor(score);
    rewindTo(cursor, 0, 0);
    addTuplet(score, cursor, Fraction(8, 5), Fraction(5, 8));

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();
    expectTuplet(score, Fraction(0, 1), Fraction(8, 5), Fraction(5, 8));

    // the last three eighths of m1 are kept
    ChordRest* next = chordRestAt(score, Fraction(5, 8));
    ASSERT_TRUE(next);
    EXPECT_TRUE(next->isChord());
    EXPECT_FALSE(next->tuplet());

    expectValidAfterReload(score, u"cursor-addTuplet-longer.mscx");

    delete score;
}

// 3 sixteenths in the time of 5 on the half note of m2: the tuplet is shorter than the chord
TEST(Engraving_ApiCursorTests, addTupletShorterThanChordRest)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);

    apiv1::Cursor cursor(score);
    rewindTo(cursor, 1, 0);
    addTuplet(score, cursor, Fraction(3, 5), Fraction(5, 16));

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();
    expectTuplet(score, Fraction(1, 1), Fraction(3, 5), Fraction(5, 16));

    // the rest of the half note's time is filled with rests
    ChordRest* next = chordRestAt(score, Fraction(1, 1) + Fraction(5, 16));
    ASSERT_TRUE(next);
    EXPECT_TRUE(next->isRest());

    expectValidAfterReload(score, u"cursor-addTuplet-shorter.mscx");

    delete score;
}

// 8 eighths in the time of 5 on a full measure rest
TEST(Engraving_ApiCursorTests, addTupletOnMeasureRest)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);

    // an empty m5
    score->startCmd(TranslatableString::untranslatable("Engraving API cursor tests"));
    score->appendMeasures(1);
    score->endCmd();
    Measure* m5 = score->lastMeasure();
    ChordRest* measureRest = chordRestAt(score, m5->tick());
    ASSERT_TRUE(measureRest && measureRest->isRest());

    apiv1::Cursor cursor(score);
    rewindTo(cursor, 4, 0);
    addTuplet(score, cursor, Fraction(8, 5), Fraction(5, 8));

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();
    expectTuplet(score, m5->tick(), Fraction(8, 5), Fraction(5, 8));

    expectValidAfterReload(score, u"cursor-addTuplet-measureRest.mscx");

    delete score;
}

// 3 eighths in the time of 5 inside the 9:8 tuplet of m3
TEST(Engraving_ApiCursorTests, addTupletNested)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);

    apiv1::Cursor cursor(score);
    rewindTo(cursor, 2, 0);
    Tuplet* parent = chordRestAt(score, Fraction(2, 1))->tuplet();
    ASSERT_TRUE(parent);
    addTuplet(score, cursor, Fraction(3, 5), Fraction(5, 8));

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();
    expectTuplet(score, Fraction(2, 1), Fraction(3, 5), Fraction(5, 8), parent);

    expectValidAfterReload(score, u"cursor-addTuplet-nested.mscx");

    delete score;
}

// a triplet whose total is a single note value works as before
TEST(Engraving_ApiCursorTests, addTupletSingleNoteValue)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);

    apiv1::Cursor cursor(score);
    rewindTo(cursor, 0, 0);
    addTuplet(score, cursor, Fraction(3, 2), Fraction(1, 4));

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();
    expectTuplet(score, Fraction(0, 1), Fraction(3, 2), Fraction(1, 4));

    expectValidAfterReload(score, u"cursor-addTuplet-single.mscx");

    delete score;
}

// notes added after addTuplet() fill the tuplet, as plugins do
TEST(Engraving_ApiCursorTests, addTupletThenAddNotes)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);

    apiv1::Cursor cursor(score);
    rewindTo(cursor, 0, 0);

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

    expectValidAfterReload(score, u"cursor-addTuplet-notes.mscx");

    delete score;
}

// the tuplet is undone in one step
TEST(Engraving_ApiCursorTests, addTupletUndo)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);

    apiv1::Cursor cursor(score);
    rewindTo(cursor, 0, 0);
    addTuplet(score, cursor, Fraction(8, 5), Fraction(5, 8));
    ASSERT_TRUE(chordRestAt(score, Fraction(0, 1))->tuplet());

    score->undoRedo(true, nullptr);

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();
    ChordRest* first = chordRestAt(score, Fraction(0, 1));
    ASSERT_TRUE(first);
    EXPECT_FALSE(first->tuplet());
    EXPECT_EQ(first->ticks(), Fraction(1, 8));

    delete score;
}

// invalid tuplets leave the score unchanged
TEST(Engraving_ApiCursorTests, addTupletInvalid)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);

    apiv1::Cursor cursor(score);

    // would cross the barline
    rewindTo(cursor, 0, 4);
    addTuplet(score, cursor, Fraction(8, 5), Fraction(5, 8));
    // the base note value (5/16) isn't a single note value
    rewindTo(cursor, 0, 0);
    addTuplet(score, cursor, Fraction(3, 2), Fraction(5, 8));

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();
    for (Segment* s = score->firstMeasure()->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        ChordRest* cr = toChordRest(s->element(0));
        EXPECT_FALSE(cr->tuplet());
        EXPECT_EQ(cr->ticks(), Fraction(1, 8));
    }

    delete score;
}

// a tuplet that would cross the end of the tuplet it starts in is rejected
TEST(Engraving_ApiCursorTests, addTupletCrossingParentTuplet)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);

    // a quintuplet over the first eighth of m4
    apiv1::Cursor cursor(score);
    rewindTo(cursor, 3, 0);
    addTuplet(score, cursor, Fraction(5, 4), Fraction(1, 8));
    const Fraction m4Tick(3, 1);
    Tuplet* quintuplet = chordRestAt(score, m4Tick)->tuplet();
    ASSERT_TRUE(quintuplet);

    // 3/8 from its second note would end after the quintuplet
    rewindTo(cursor, 3, 1);
    addTuplet(score, cursor, Fraction(2, 3), Fraction(3, 8));

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();
    ChordRest* second = chordRestAt(score, m4Tick + Fraction(1, 40));
    ASSERT_TRUE(second);
    EXPECT_EQ(second->tuplet(), quintuplet);
    EXPECT_EQ(second->ticks(), Fraction(1, 32));

    expectValidAfterReload(score, u"cursor-addTuplet-crossingParent.mscx");

    delete score;
}
