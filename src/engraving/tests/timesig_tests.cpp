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

#include "dom/factory.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/timesig.h"
#include "dom/undo.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String TIMESIG_DATA_DIR(u"timesig_data/");

class Engraving_TimesigTests : public ::testing::Test
{
};

//---------------------------------------------------------
///   timesig01
///   add a 3/4 time signature in the second measure
//---------------------------------------------------------

TEST_F(Engraving_TimesigTests, timesig01)
{
    MasterScore* score = ScoreRW::readScore(TIMESIG_DATA_DIR + "timesig01.mscx");
    EXPECT_TRUE(score);
    Measure* m  = score->firstMeasure()->nextMeasure();
    TimeSig* ts = Factory::createTimeSig(score->dummy()->segment());
    ts->setSig(Fraction(3, 4), TimeSigType::NORMAL);

    score->startCmd(TranslatableString::untranslatable("Engraving time signature tests"));
    int staffIdx = 0;
    bool local   = false;
    score->cmdAddTimeSig(m, staffIdx, ts, local);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"timesig01.mscx", TIMESIG_DATA_DIR + u"timesig01-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///   timesig02
///   Attempt to change a 4/4 measure containing a triplet of minims to a 3/4 time signature
///   The attempt should fail, the score left unchanged
//---------------------------------------------------------

TEST_F(Engraving_TimesigTests, timesig02)
{
    MasterScore* score = ScoreRW::readScore(TIMESIG_DATA_DIR + "timesig-02.mscx");
    EXPECT_TRUE(score);
    Measure* m = score->firstMeasure();
    TimeSig* ts = Factory::createTimeSig(score->dummy()->segment());
    ts->setSig(Fraction(3, 4), TimeSigType::NORMAL);

    score->startCmd(TranslatableString::untranslatable("Engraving time signature tests"));
    score->cmdAddTimeSig(m, 0, ts, false);
    score->doLayout();
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"timesig-02.mscx", TIMESIG_DATA_DIR + u"timesig-02-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///   timesig03
///   add a 3/4 time signature in the second measure
///   rewrite notes
///   be sure that annotations and spanners are preserved
///   even annotations in otherwise empty segments
///   also measure repeats and non-default barlines
//---------------------------------------------------------

TEST_F(Engraving_TimesigTests, timesig03)
{
    MasterScore* score = ScoreRW::readScore(TIMESIG_DATA_DIR + u"timesig-03.mscx");
    EXPECT_TRUE(score);
    Measure* m = score->firstMeasure()->nextMeasure();
    TimeSig* ts = Factory::createTimeSig(score->dummy()->segment());
    ts->setSig(Fraction(3, 4), TimeSigType::NORMAL);

    score->cmdAddTimeSig(m, 0, ts, false);
    score->doLayout();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"timesig-03.mscx", TIMESIG_DATA_DIR + u"timesig-03-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///   timesig04
///   add a 6/4 time signature in the second measure
///   which already contains a quarter note
//---------------------------------------------------------

TEST_F(Engraving_TimesigTests, timesig04)
{
    MasterScore* score = ScoreRW::readScore(TIMESIG_DATA_DIR + "timesig-04.mscx");
    EXPECT_TRUE(score);
    Measure* m = score->firstMeasure()->nextMeasure();
    TimeSig* ts = Factory::createTimeSig(score->dummy()->segment());
    ts->setSig(Fraction(6, 4), TimeSigType::NORMAL);

    score->cmdAddTimeSig(m, 0, ts, false);
    score->doLayout();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"timesig-04.mscx", TIMESIG_DATA_DIR + u"timesig-04-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///   timesig05
///   Add a 3/4 time signature to the first measure.
///   Test that spanners are preserved, especially those
///   that span across time signature change border.
///   Inspired by the issue #279593 where such spanners
///   caused crashes.
//---------------------------------------------------------

TEST_F(Engraving_TimesigTests, timesig05)
{
    MasterScore* score = ScoreRW::readScore(TIMESIG_DATA_DIR + "timesig-05.mscx");
    EXPECT_TRUE(score);
    Measure* m = score->firstMeasure();
    TimeSig* ts = Factory::createTimeSig(score->dummy()->segment());
    ts->setSig(Fraction(3, 4), TimeSigType::NORMAL);

    score->cmdAddTimeSig(m, 0, ts, false);
    score->doLayout();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"timesig-05.mscx", TIMESIG_DATA_DIR + u"timesig-05-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//   timesig06
//    Change timesig with a tremolo that doesn't end up across a barline
//---------------------------------------------------------

TEST_F(Engraving_TimesigTests, timesig06)
{
    MasterScore* score = ScoreRW::readScore(TIMESIG_DATA_DIR + "timesig-06.mscx");
    EXPECT_TRUE(score);
    Measure* m = score->firstMeasure();
    TimeSig* ts = Factory::createTimeSig(score->dummy()->segment());
    ts->setSig(Fraction(5, 4), TimeSigType::NORMAL);

    score->startCmd(TranslatableString::untranslatable("Engraving time signature tests"));
    score->cmdAddTimeSig(m, 0, ts, false);
    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"timesig-06.mscx", TIMESIG_DATA_DIR + u"timesig-06-ref.mscx"));
    score->endCmd();

    // Now undo the change, if it crashes, it will fail
    score->undoStack()->undo(0);
    score->doLayout();
    delete score;
}

//---------------------------------------------------------
//   timesig07
//    Change timesig with a tremolo that _does_ end up across a barline
//    The tremolo should end up removed.
//---------------------------------------------------------

TEST_F(Engraving_TimesigTests, timesig07)
{
    MasterScore* score = ScoreRW::readScore(TIMESIG_DATA_DIR + u"timesig-07.mscx");
    EXPECT_TRUE(score);
    Measure* m = score->firstMeasure();
    TimeSig* ts = Factory::createTimeSig(score->dummy()->segment());
    ts->setSig(Fraction(3, 4), TimeSigType::NORMAL);

    score->startCmd(TranslatableString::untranslatable("Engraving time signature tests"));
    score->cmdAddTimeSig(m, 0, ts, false);
    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"timesig-07.mscx", TIMESIG_DATA_DIR + u"timesig-07-ref.mscx"));
    score->endCmd();

    // Now undo the change, if there is a crash the test will fail
    score->undoStack()->undo(0);
    score->doLayout();
    delete score;
}

//---------------------------------------------------------
//   timesig08
//    Check if a courtesy time signature is created along with
//    a local time signature in the next system, no matter which staff the local time signature is in
//    (in this particular case, stave no.2)
//---------------------------------------------------------

TEST_F(Engraving_TimesigTests, timesig08)
{
    MasterScore* score = ScoreRW::readScore(TIMESIG_DATA_DIR + u"timesig-08.mscx");
    score->doLayout();

    Measure* m1 = score->firstMeasure();
    Segment* seg = m1->findSegment(SegmentType::TimeSigAnnounce, m1->endTick());
    EngravingItem* el = seg->element(staff2track(1));

    EXPECT_TRUE(el) << "Should be a courtesy signature in the second staff at the end of measure 1.";
    delete score;
}

//---------------------------------------------------------
//   timesig09
//    Change timesig with tremolos on notes that end up across barlines
//---------------------------------------------------------

TEST_F(Engraving_TimesigTests, DISABLED_timesig09)
{
    MasterScore* score = ScoreRW::readScore(TIMESIG_DATA_DIR + "timesig-09.mscx");
    EXPECT_TRUE(score);
    Measure* m = score->firstMeasure();
    TimeSig* ts = Factory::createTimeSig(score->dummy()->segment());
    ts->setSig(Fraction(9, 8), TimeSigType::NORMAL);

    score->startCmd(TranslatableString::untranslatable("Engraving time signature tests"));
    score->cmdAddTimeSig(m, 0, ts, false);
    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"timesig-09-1.mscx", TIMESIG_DATA_DIR + u"timesig-09-ref.mscx"));
    score->endCmd();

    // Now undo the change
    score->undoStack()->undo(0);
    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"timesig-09-2.mscx", TIMESIG_DATA_DIR + u"timesig-09.mscx"));
    delete score;
}

//---------------------------------------------------------
//   timesig10
//    Check if 4/4 is correctly changed to alla breve when commanded to do so
//    Same for 2/2 to common time
//---------------------------------------------------------

TEST_F(Engraving_TimesigTests, timesig10)
{
    MasterScore* score = ScoreRW::readScore(TIMESIG_DATA_DIR + u"timesig-10.mscx");

    Measure* m1 = score->firstMeasure();
    TimeSig* ts1 = Factory::createTimeSig(score->dummy()->segment());
    ts1->setSig(Fraction(2, 2), TimeSigType::ALLA_BREVE);

    score->startCmd(TranslatableString::untranslatable("Engraving time signature tests"));
    score->cmdAddTimeSig(m1, 0, ts1, false);

    Measure* m2 = m1->nextMeasure();
    TimeSig* ts2 = Factory::createTimeSig(score->dummy()->segment());
    ts2->setSig(Fraction(2, 2), TimeSigType::NORMAL);
    TimeSig* ts3 = Factory::createTimeSig(score->dummy()->segment());
    ts3->setSig(Fraction(4, 4), TimeSigType::FOUR_FOUR);

    score->cmdAddTimeSig(m2, 0, ts2, false);
    m2 = score->firstMeasure()->nextMeasure();
    score->cmdAddTimeSig(m2, 0, ts3, false);

    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"timesig-10.mscx", TIMESIG_DATA_DIR + u"timesig-10-ref.mscx"));
    score->endCmd();
    delete score;
}

//---------------------------------------------------------
//   timesig_78216
//    input score has section breaks on non-measure MeasureBase objects.
//    should not display courtesy timesig at the end of final measure of each section (meas 1, 2, & 3), even if section break occurs on subsequent non-measure frame.
//---------------------------------------------------------

TEST_F(Engraving_TimesigTests, timesig_78216)
{
    MasterScore* score = ScoreRW::readScore(TIMESIG_DATA_DIR + "timesig_78216.mscx");
    score->doLayout();

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();
    Measure* m3 = m2->nextMeasure();

    // verify no timesig exists in segment of final tick of m1, m2, m3
    EXPECT_FALSE(m1->findSegment(SegmentType::TimeSig, m1->endTick())) << "Should be no timesig at the end of measure 1.";
    EXPECT_FALSE(m2->findSegment(SegmentType::TimeSig, m2->endTick())) << "Should be no timesig at the end of measure 2.";
    EXPECT_FALSE(m3->findSegment(SegmentType::TimeSig, m3->endTick())) << "Should be no timesig at the end of measure 3.";
    delete score;
}
