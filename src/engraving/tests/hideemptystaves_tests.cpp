/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include <utility>

#include "dom/masterscore.h"
#include "dom/part.h"
#include "dom/staff.h"

#include "types/types.h"
#include "utils/scorerw.h"

using namespace mu::engraving;

static const String HIDEEMPTYSTAVES_DATA_DIR("hideemptystaves_data/");

class Engraving_HideEmptyStavesTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_useRead302 = std::exchange(MScore::useRead302InTestMode, false);
    }

    void TearDown() override
    {
        MScore::useRead302InTestMode = m_useRead302;
    }

private:
    bool m_useRead302 = false;
};

TEST_F(Engraving_HideEmptyStavesTests, Compat450)
{
    MasterScore* masterScore = ScoreRW::readScore(HIDEEMPTYSTAVES_DATA_DIR + "compat450/compat450.mscx");
    ASSERT_TRUE(masterScore);

    // 4 single-staff parts and 10 two-staff parts
    ASSERT_EQ(masterScore->parts().size(), 4 + 10);
    ASSERT_EQ(masterScore->staves().size(), 4 * 1 + 10 * 2);

    // global hide empty staves is on
    EXPECT_TRUE(masterScore->style().styleB(Sid::hideEmptyStaves));

    auto p = [masterScore](size_t idx) {
        return masterScore->parts().at(idx);
    };
    auto ps = [masterScore](size_t partIdx, size_t staffInPartIdx) {
        return masterScore->parts().at(partIdx)->staves().at(staffInPartIdx);
    };

    // Single-staff parts
    // auto
    EXPECT_EQ(p(0)->hideWhenEmpty(), AutoOnOff::AUTO);
    EXPECT_FALSE(p(0)->hideStavesWhenIndividuallyEmpty());
    EXPECT_EQ(ps(0, 0)->hideWhenEmpty(), AutoOnOff::AUTO);

    // always
    EXPECT_EQ(p(1)->hideWhenEmpty(), AutoOnOff::ON);
    EXPECT_FALSE(p(1)->hideStavesWhenIndividuallyEmpty());
    EXPECT_EQ(ps(1, 0)->hideWhenEmpty(), AutoOnOff::AUTO);

    // never
    EXPECT_EQ(p(2)->hideWhenEmpty(), AutoOnOff::OFF);
    EXPECT_FALSE(p(2)->hideStavesWhenIndividuallyEmpty());
    EXPECT_EQ(ps(2, 0)->hideWhenEmpty(), AutoOnOff::AUTO);

    // instrument
    EXPECT_EQ(p(3)->hideWhenEmpty(), AutoOnOff::AUTO);
    EXPECT_FALSE(p(3)->hideStavesWhenIndividuallyEmpty());
    EXPECT_EQ(ps(3, 0)->hideWhenEmpty(), AutoOnOff::AUTO);

    // Two-staff parts
    // auto auto
    EXPECT_EQ(p(4)->hideWhenEmpty(), AutoOnOff::AUTO);
    EXPECT_TRUE(p(4)->hideStavesWhenIndividuallyEmpty());
    EXPECT_EQ(ps(4, 0)->hideWhenEmpty(), AutoOnOff::AUTO);
    EXPECT_EQ(ps(4, 1)->hideWhenEmpty(), AutoOnOff::AUTO);

    // auto always
    EXPECT_EQ(p(5)->hideWhenEmpty(), AutoOnOff::AUTO);
    EXPECT_TRUE(p(5)->hideStavesWhenIndividuallyEmpty());
    EXPECT_EQ(ps(5, 0)->hideWhenEmpty(), AutoOnOff::AUTO);
    EXPECT_EQ(ps(5, 1)->hideWhenEmpty(), AutoOnOff::ON);

    // auto never
    EXPECT_EQ(p(6)->hideWhenEmpty(), AutoOnOff::AUTO);
    EXPECT_TRUE(p(6)->hideStavesWhenIndividuallyEmpty());
    EXPECT_EQ(ps(6, 0)->hideWhenEmpty(), AutoOnOff::AUTO);
    EXPECT_EQ(ps(6, 1)->hideWhenEmpty(), AutoOnOff::OFF);

    // auto instrument
    EXPECT_EQ(p(7)->hideWhenEmpty(), AutoOnOff::AUTO);
    EXPECT_FALSE(p(7)->hideStavesWhenIndividuallyEmpty());
    EXPECT_EQ(ps(7, 0)->hideWhenEmpty(), AutoOnOff::AUTO);
    EXPECT_EQ(ps(7, 1)->hideWhenEmpty(), AutoOnOff::AUTO);

    // always always
    EXPECT_EQ(p(8)->hideWhenEmpty(), AutoOnOff::ON);
    EXPECT_TRUE(p(8)->hideStavesWhenIndividuallyEmpty());
    EXPECT_EQ(ps(8, 0)->hideWhenEmpty(), AutoOnOff::AUTO);
    EXPECT_EQ(ps(8, 1)->hideWhenEmpty(), AutoOnOff::AUTO);

    // always never
    EXPECT_EQ(p(9)->hideWhenEmpty(), AutoOnOff::AUTO);
    EXPECT_FALSE(p(9)->hideStavesWhenIndividuallyEmpty());
    EXPECT_EQ(ps(9, 0)->hideWhenEmpty(), AutoOnOff::ON);
    EXPECT_EQ(ps(9, 1)->hideWhenEmpty(), AutoOnOff::OFF);

    // always instrument
    EXPECT_EQ(p(10)->hideWhenEmpty(), AutoOnOff::AUTO);
    EXPECT_FALSE(p(10)->hideStavesWhenIndividuallyEmpty());
    EXPECT_EQ(ps(10, 0)->hideWhenEmpty(), AutoOnOff::ON);
    EXPECT_EQ(ps(10, 1)->hideWhenEmpty(), AutoOnOff::AUTO);

    // never never
    EXPECT_EQ(p(11)->hideWhenEmpty(), AutoOnOff::OFF);
    EXPECT_FALSE(p(11)->hideStavesWhenIndividuallyEmpty());
    EXPECT_EQ(ps(11, 0)->hideWhenEmpty(), AutoOnOff::AUTO);
    EXPECT_EQ(ps(11, 1)->hideWhenEmpty(), AutoOnOff::AUTO);

    // never instrument
    EXPECT_EQ(p(12)->hideWhenEmpty(), AutoOnOff::AUTO);
    EXPECT_FALSE(p(12)->hideStavesWhenIndividuallyEmpty());
    EXPECT_EQ(ps(12, 0)->hideWhenEmpty(), AutoOnOff::OFF);
    EXPECT_EQ(ps(12, 1)->hideWhenEmpty(), AutoOnOff::AUTO);

    // instrument instrument
    EXPECT_EQ(p(13)->hideWhenEmpty(), AutoOnOff::AUTO);
    EXPECT_FALSE(p(13)->hideStavesWhenIndividuallyEmpty());
    EXPECT_EQ(ps(13, 0)->hideWhenEmpty(), AutoOnOff::AUTO);
    EXPECT_EQ(ps(13, 1)->hideWhenEmpty(), AutoOnOff::AUTO);

    delete masterScore;
}
