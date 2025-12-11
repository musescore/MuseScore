/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "engraving/automation/internal/automation.h"
#include "engraving/dom/staff.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

using InterpolationType = AutomationPoint::InterpolationType;

static const String AUTOMATION_DATA_DIR(u"automation/data/");

static constexpr double P_VALUE(0.429);
static constexpr double MID_VALUE(0.5);
static constexpr double MP_VALUE(0.5);
static constexpr double F_VALUE(0.643);

class Engraving_AutomationTests : public ::testing::Test
{
};

TEST_F(Engraving_AutomationTests, Init_Dynamics)
{
    // [GIVEN] Score with dynamics
    Score* score = ScoreRW::readScore(AUTOMATION_DATA_DIR + u"dynamics.mscx");
    ASSERT_TRUE(score);
    ASSERT_FALSE(score->staves().empty());

    // [WHEN] Calculate the dynamics curve
    Automation automation;
    automation.init(score);

    // [THEN] Curve matches expectations
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = score->staff(0)->id();

    const AutomationCurve& actualCurve = automation.curve(key);

    // 1st measure
    AutomationCurve expectedCurve;
    expectedCurve[480] = AutomationPoint { MID_VALUE, P_VALUE, InterpolationType::Linear }; // 2nd beat: p
    expectedCurve[1440] = AutomationPoint { P_VALUE, MP_VALUE, InterpolationType::Linear }; // 4th beat: mp

    // 2nd measure
    expectedCurve[1920] = AutomationPoint { MP_VALUE, F_VALUE, InterpolationType::Linear }; // 1st beat: sf
    expectedCurve[2400] = AutomationPoint { F_VALUE, MP_VALUE, InterpolationType::Linear }; // 2nd beat: mp
    expectedCurve[2880] = AutomationPoint { MP_VALUE, P_VALUE, InterpolationType::Exponential }; // 3rd beat: p (pf)
    expectedCurve[3264] = AutomationPoint { P_VALUE, F_VALUE, InterpolationType::Linear }; // 4th beat: f (pf)

    EXPECT_EQ(actualCurve.size(), expectedCurve.size());

    for (const auto& [utick, actualPoint] : actualCurve) {
        ASSERT_TRUE(muse::contains(expectedCurve, utick));
        const AutomationPoint& expectedPoint = expectedCurve.at(utick);

        EXPECT_NEAR(actualPoint.inValue, expectedPoint.inValue, 0.0001);
        EXPECT_NEAR(actualPoint.outValue, expectedPoint.outValue, 0.0001);
        EXPECT_EQ(actualPoint.interpolation, expectedPoint.interpolation);
    }

    delete score;
}
