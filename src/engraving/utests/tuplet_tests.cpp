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

#include "libmscore/factory.h"
#include "libmscore/staff.h"
#include "libmscore/masterscore.h"
#include "libmscore/tuplet.h"
#include "libmscore/chord.h"
#include "libmscore/measure.h"
#include "libmscore/timesig.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

static const QString TUPLET_DATA_DIR("tuplet_data/");

using namespace mu::engraving;
using namespace Ms;

class TupletTests : public ::testing::Test
{
public:
    bool createTuplet(int n, ChordRest* cr);
    void tuplet(const char* p1, const char* p2);
    void split(const char* p1, const char* p2);
};

bool TupletTests::createTuplet(int n, ChordRest* cr)
{
    if (cr->durationType() < TDuration(DurationType::V_128TH)) {
        return false;
    }

    Fraction f(cr->ticks());
    Fraction tick = cr->tick();
    Tuplet* ot    = cr->tuplet();

    f.reduce();         //measure duration might not be reduced
    Fraction ratio(n, f.numerator());
    Fraction fr(1, f.denominator());
    while (ratio.numerator() >= ratio.denominator() * 2) {
        ratio *= Fraction(1, 2);
        fr    *= Fraction(1, 2);
    }

    Tuplet* tuplet = Factory::createTuplet(cr->score()->dummy()->measure());
    tuplet->setRatio(ratio);

    //
    // "fr" is the fraction value of one tuple element
    //
    // "tuplet time" is "normal time" / tuplet->ratio()
    //    Example: an 1/8 has 240 midi ticks, in an 1/8 triplet the note
    //             has a tick duration of 240 / (3/2) = 160 ticks
    //             (assume tpq = 480)
    //

    tuplet->setTicks(f);
    TDuration baseLen(fr);
    tuplet->setBaseLen(baseLen);

    tuplet->setTrack(cr->track());
    tuplet->setTick(tick);
    Measure* measure = cr->measure();
    tuplet->setParent(measure);

    if (ot) {
        tuplet->setTuplet(ot);
    }
    cr->score()->startCmd();
    cr->score()->cmdCreateTuplet(cr, tuplet);
    cr->score()->endCmd();
    return true;
}

void TupletTests::tuplet(const char* p1, const char* p2)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + p1);
    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();

    EXPECT_TRUE(m1 != 0);
    EXPECT_TRUE(m2 != 0);
    EXPECT_TRUE(m1 != m2);

    Segment* s = m2->first(SegmentType::ChordRest);
    EXPECT_TRUE(s != 0);
    Ms::Chord* c = toChord(s->element(0));
    EXPECT_TRUE(c != 0);

    EXPECT_TRUE(createTuplet(3, c));

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, p1, TUPLET_DATA_DIR + p2));
    delete score;
}

TEST_F(TupletTests, join1)
{
    tuplet("tuplet1.mscx", "tuplet1-ref.mscx");
}

void TupletTests::split(const char* p1, const char* p2)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + p1);
    Measure* m         = score->firstMeasure();
    TimeSig* ts        = Factory::createTimeSig(score->dummy()->segment());
    ts->setSig(Fraction(3, 4), TimeSigType::NORMAL);

    score->startCmd();
    EditData dd(0);
    dd.dropElement = ts;
    dd.modifiers = {};
    dd.dragOffset = QPointF();
    dd.pos = m->pagePos();
    m->drop(dd);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, p1, TUPLET_DATA_DIR + p2));
    delete score;
}

TEST_F(TupletTests, split1)
{
    split("split1.mscx",   "split1-ref.mscx");
}

TEST_F(TupletTests, split2)
{
    split("split2.mscx",   "split2-ref.mscx");
}

TEST_F(TupletTests, split3)
{
    split("split3.mscx",   "split3-ref.mscx");
}

TEST_F(TupletTests, split4)
{
    split("split4.mscx",   "split4-ref.mscx");
}

//---------------------------------------------------------
//   addStaff
//    Checks that after adding a staff the resulting
//    score is equal to the reference score
//---------------------------------------------------------

TEST_F(TupletTests, addStaff)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + "nestedTuplets_addStaff.mscx");
    EXPECT_TRUE(score);

    // add a staff to the existing staff
    // (copied and adapted from void MuseScore::editInstrList() in mscore/instrdialog.cpp)
    Staff* oldStaff   = score->staff(0);
    Staff* newStaff   = Factory::createStaff(oldStaff->part());
    newStaff->setPart(oldStaff->part());
    newStaff->initFromStaffType(oldStaff->staffType(Fraction(0, 1)));
    newStaff->setDefaultClefType(ClefTypeList(ClefType::F));
    KeySigEvent ke = oldStaff->keySigEvent(Fraction(0, 1));
    newStaff->setKey(Fraction(0, 1), ke);
    score->undoInsertStaff(newStaff, 0, true);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, "nestedTuplets_addStaff.mscx", TUPLET_DATA_DIR + "nestedTuplets_addStaff-ref.mscx"));
    delete score;
}

//-----------------------------------------
//    saveLoad
//     checks that properties persist after loading and saving
//-----------------------------------------
TEST_F(TupletTests, saveLoad)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + "save-load.mscx");
    EXPECT_TRUE(score);
    //simply load and save
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, "save-load.mscx", TUPLET_DATA_DIR + "save-load.mscx"));
    delete score;
}
