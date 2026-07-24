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

#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/timesig.h"

#include "engraving/editing/editclef.h"
#include "engraving/editing/edittimesig.h"
#include "engraving/editing/transaction/transaction.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu::engraving;

static const String CLEF_DATA_DIR(u"clef_data/");

class Engraving_ClefTests : public ::testing::Test
{
};

//---------------------------------------------------------
//    two clefs at tick position zero
//---------------------------------------------------------
TEST_F(Engraving_ClefTests, clef1)
{
    MasterScore* score = ScoreRW::readScore(CLEF_DATA_DIR + u"clef-1.mscx");
    EXPECT_TRUE(score);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"clef-1.mscx", CLEF_DATA_DIR + u"clef-1-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//    change timesig -> rewrite measures ->insertTime
//---------------------------------------------------------
TEST_F(Engraving_ClefTests, clef2)
{
    MasterScore* score = ScoreRW::readScore(CLEF_DATA_DIR + u"clef-2.mscx");
    ASSERT_TRUE(score);

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Clef tests"), [&](Transaction& tx) {
        Measure* m = score->firstMeasure();
        m = m->nextMeasure();
        m = m->nextMeasure();
        TimeSig* ts = Factory::createTimeSig(score->dummy()->segment());
        ts->setSig(Fraction(2, 4));
        EditTimeSig::addTimeSig(tx, score, m, 0, ts, false);
    });

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"clef-2.mscx", CLEF_DATA_DIR + u"clef-2-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//    change the first clef of a score by changing the first measure's clef
//---------------------------------------------------------
TEST_F(Engraving_ClefTests, clef3)
{
    MasterScore* score = ScoreRW::readScore(CLEF_DATA_DIR + u"clef-3.mscx");
    ASSERT_TRUE(score);

    score->transactionManager()->transaction(muse::TranslatableString::untranslatable("Clef tests"), [&](Transaction& tx) {
        Measure* m = score->firstMeasure();
        EditClef::undoChangeClef(tx, score, score->staff(0), m, ClefType::F);
    });

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"clef-3.mscx", CLEF_DATA_DIR + u"clef-3-ref.mscx"));
    delete score;
}
