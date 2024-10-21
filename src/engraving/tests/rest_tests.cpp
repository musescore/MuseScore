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

#include "dom/measure.h"
#include "dom/rest.h"

#include "utils/scorerw.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

static const String REST_DATA_DIR(u"rest_data/");
static const int TICKS_PER_4_2_MEASURE = 8 * 480; // 4/2 time per measure tick (8 quarters)

class Engraving_RestTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        //! NOTE: allows to read test files using their version readers
        //! instead of using 302 (see mscloader.cpp, makeReader)
        MScore::useRead302InTestMode = false;
    }

    void TearDown() override
    {
        MScore::useRead302InTestMode = true;
    }

    Rest* findRest(const MasterScore* score, int tick) const
    {
        IF_ASSERT_FAILED(score) {
            return nullptr;
        }

        ChordRest* cr = score->findCR(Fraction::fromTicks(tick), 0);
        if (cr->isRest()) {
            return toRest(cr);
        }

        return nullptr;
    }

public:
};

TEST_F(Engraving_RestTests, BreveRests_LedgerLinesNormalStaff)
{
    MasterScore* score = ScoreRW::readScore(REST_DATA_DIR + u"rest01.mscz");
    ASSERT_TRUE(score);

    // [GIVEN] Style setting for breve rests is with ledger lines
    score->style().set(Sid::showLedgerLinesOnBreveRests, true);
    score->doLayout();

    // [GIVEN] Breve fullmeasure rests in each bar
    for (int measureNum = 1; measureNum <= score->measures()->size(); measureNum++) {
        Rest* rest = findRest(score, (measureNum - 1) * TICKS_PER_4_2_MEASURE);
        // [THEN] ledger lines on 2 & 3
        SymId expectedSym = (measureNum == 2) || (measureNum == 3)
                            ? SymId::restDoubleWholeLegerLine
                            : SymId::restDoubleWhole;
        EXPECT_EQ(rest->ldata()->sym, expectedSym);
    }

    // [GIVEN] Style setting for breve rests is with no ledger lines
    score->style().set(Sid::showLedgerLinesOnBreveRests, false);
    score->doLayout();

    // [GIVEN] Breve fullmeasure rests in each bar
    for (int measureNum = 1; measureNum <= score->measures()->size(); measureNum++) {
        Rest* rest = findRest(score, (measureNum - 1) * TICKS_PER_4_2_MEASURE);
        // [THEN] no bars should have ledger lines
        EXPECT_EQ(rest->ldata()->sym, SymId::restDoubleWhole);
    }

    delete score;
}

TEST_F(Engraving_RestTests, BreveRests_LedgerLinesOneLineStaff)
{
    MasterScore* score = ScoreRW::readScore(REST_DATA_DIR + u"rest02.mscz");
    ASSERT_TRUE(score);

    // [GIVEN] Style setting for breve rests is with ledger lines
    score->style().set(Sid::showLedgerLinesOnBreveRests, true);
    score->doLayout();

    // [GIVEN] Breve fullmeasure rests in each bar
    for (int measureNum = 1; measureNum <= score->measures()->size(); measureNum++) {
        Rest* rest = findRest(score, (measureNum - 1) * TICKS_PER_4_2_MEASURE);
        // [THEN] ledger lines on all bars
        EXPECT_EQ(rest->ldata()->sym, SymId::restDoubleWholeLegerLine);
    }

    // [GIVEN] Style setting for breve rests is with no ledger lines
    score->style().set(Sid::showLedgerLinesOnBreveRests, false);
    score->doLayout();

    // [GIVEN] Breve fullmeasure rests in each bar
    for (int measureNum = 1; measureNum <= score->measures()->size(); measureNum++) {
        Rest* rest = findRest(score, (measureNum - 1) * TICKS_PER_4_2_MEASURE);
        // [THEN] no bars should have ledger lines
        EXPECT_EQ(rest->ldata()->sym, SymId::restDoubleWhole);
    }

    delete score;
}
