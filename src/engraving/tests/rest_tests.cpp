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

#include "dom/rest.h"
#include "dom/staff.h"

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

    Rest* findRest(const MasterScore* score, int tick, track_idx_t track = 0) const
    {
        IF_ASSERT_FAILED(score) {
            return nullptr;
        }

        ChordRest* cr = score->findCR(Fraction::fromTicks(tick), track);
        if (cr->isRest()) {
            return toRest(cr);
        }

        return nullptr;
    }

    void testRestLines(MasterScore* score, std::function<int(int)> calcTickFromMeasureNum,
                       const std::vector<int>& expectedLines, const std::vector<track_idx_t>& restTracks)
    {
        ASSERT_EQ(expectedLines.size(), restTracks.size());
        ASSERT_TRUE(score->measures()->size() >= static_cast<int>(restTracks.size()));
        // [GIVEN] fullmeasure rests in each bar
        for (int measureNum = 1; measureNum <= static_cast<int>(restTracks.size()); measureNum++) {
            Rest* rest = findRest(score, calcTickFromMeasureNum(measureNum), restTracks[measureNum - 1]);
            ASSERT_TRUE(rest);
            // [THEN] line numbers match on all bars
            int visibleLine = std::floor(rest->pos().y() / (rest->staff()->lineDistance(rest->tick()) * rest->spatium()));
            EXPECT_EQ(visibleLine, expectedLines[measureNum - 1]);
        }
    }

public:
};

TEST_F(Engraving_RestTests, BreveRests_TestFullmeasure1Line)
{
    MasterScore* score = ScoreRW::readScore(REST_DATA_DIR + u"rest03.mscz");
    ASSERT_TRUE(score);
    score->doLayout();

    auto calcTick = [](int measureNum) -> int {
        // 1 bar of 4/2 followed by 1 bar of 4/4
        return std::max(0, measureNum - 1) * TICKS_PER_4_2_MEASURE;
    };

    std::vector<int> expectedLines = { 1, 0 };
    testRestLines(score, calcTick, expectedLines, { 0, 0 });

    delete score;
}
