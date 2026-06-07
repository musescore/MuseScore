/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
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

#include <memory>

#include "engraving/automation/internal/automationcontroller.h"
#include "engraving/automation/iautomation.h"
#include "engraving/dom/staff.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

using InterpolationType = AutomationPoint::InterpolationType;

static const String AUTOMATION_DATA_DIR(u"automation/data/");

static constexpr double P_VALUE(0.429);
static constexpr double MID_VALUE(0.5);
static constexpr double MP_VALUE(0.5);
static constexpr double F_VALUE(0.643);
static constexpr double FF_VALUE(0.714);

class Engraving_AutomationTests : public ::testing::Test
{
};

static AutomationPoint makePoint(double inValue, double outValue,
                                 InterpolationType interpolation = InterpolationType::Linear)
{
    return AutomationPoint { inValue, outValue, interpolation, std::nullopt };
}

static void checkCurvesMatch(const AutomationCurve& actualCurve, const AutomationCurve& expectedCurve)
{
    EXPECT_EQ(actualCurve.size(), expectedCurve.size());

    for (const auto& [utick, actualPoint] : actualCurve) {
        ASSERT_TRUE(muse::contains(expectedCurve, utick));
        const AutomationPoint& expectedPoint = expectedCurve.at(utick);

        EXPECT_NEAR(actualPoint.inValue, expectedPoint.inValue, 0.0001);
        EXPECT_NEAR(actualPoint.outValue, expectedPoint.outValue, 0.0001);
        EXPECT_EQ(actualPoint.interpolation, expectedPoint.interpolation);
    }
}

TEST_F(Engraving_AutomationTests, Init_Dynamics)
{
    // [GIVEN] Score with dynamics
    std::unique_ptr<Score> score(ScoreRW::readScore(AUTOMATION_DATA_DIR + u"dynamics.mscx"));
    ASSERT_TRUE(score);
    ASSERT_FALSE(score->staves().empty());

    // [WHEN] Calculate the dynamics curve
    AutomationController controller;
    controller.init(score.get());

    // [THEN] Curve matches expectations
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = score->staff(0)->id();

    AutomationCurve actualCurve = controller.automation()->curve(key);

    // 1st measure
    AutomationCurve expectedCurve;
    expectedCurve[480] = makePoint(MID_VALUE, P_VALUE); // 2nd beat: p
    expectedCurve[1440] = makePoint(P_VALUE, MP_VALUE); // 4th beat: mp

    // 2nd measure
    expectedCurve[1920] = makePoint(MP_VALUE, F_VALUE); // 1st beat: sf
    expectedCurve[2400] = makePoint(F_VALUE, MP_VALUE); // 2nd beat: mp
    expectedCurve[2880] = makePoint(MP_VALUE, P_VALUE, InterpolationType::Exponential); // 3rd beat: p (pf)
    expectedCurve[3264] = makePoint(P_VALUE, F_VALUE); // 4th beat: f (pf)

    // 3rd measure
    expectedCurve[4800] = makePoint(F_VALUE, P_VALUE); // 3rd beat: p (hairpin starts)

    // 4th measure
    expectedCurve[5760] = makePoint(FF_VALUE, FF_VALUE); // 1st beat: ff (hairpin ends)

    checkCurvesMatch(actualCurve, expectedCurve);

    // [THEN] 2nd voice curve matches expectations
    key.voiceIdx = 1;
    actualCurve = controller.automation()->curve(key);

    // 3rd measure
    expectedCurve.clear();
    expectedCurve[3840] = makePoint(MID_VALUE, F_VALUE); // 1st beat: f (applied only to 2nd voice)

    checkCurvesMatch(actualCurve, expectedCurve);

}

TEST_F(Engraving_AutomationTests, ReadWrite_ExpressionStaffId)
{
    std::unique_ptr<Score> score(ScoreRW::readScore(AUTOMATION_DATA_DIR + u"dynamics.mscx"));
    ASSERT_TRUE(score);
    ASSERT_FALSE(score->staves().empty());

    AutomationController controller;
    controller.init(score.get());

    AutomationCurveKey key;
    key.type = AutomationType::Expression;
    key.staffId = score->staff(0)->id();

    AutomationPoint point;
    point.inValue = 0.25;
    point.outValue = 0.75;
    point.interpolation = InterpolationType::Linear;
    controller.automation()->addPoint(key, 480, point);

    AutomationController roundTripController;
    roundTripController.automation()->read(controller.automation()->toJson());

    AutomationCurve expectedCurve;
    expectedCurve[480] = point;
    checkCurvesMatch(roundTripController.automation()->curve(key), expectedCurve);

}
