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

#include "engraving/automation/internal/automation.h"
#include "global/containers.h"

using namespace mu::engraving;

using InterpolationType = AutomationPoint::InterpolationType;

static void checkCurvesMatch(const AutomationCurve& actualCurve, const AutomationCurve& expectedCurve)
{
    EXPECT_EQ(actualCurve.size(), expectedCurve.size());

    for (const auto& [utick, expectedPoint] : expectedCurve) {
        ASSERT_TRUE(muse::contains(actualCurve, utick)) << "Missing point at utick " << utick;
        const AutomationPoint& actualPoint = actualCurve.at(utick);

        EXPECT_NEAR(actualPoint.inValue, expectedPoint.inValue, 0.0001) << "inValue mismatch at utick " << utick;
        EXPECT_NEAR(actualPoint.outValue, expectedPoint.outValue, 0.0001) << "outValue mismatch at utick " << utick;
        EXPECT_EQ(actualPoint.interpolation, expectedPoint.interpolation) << "interpolation mismatch at utick " << utick;
    }
}

static AutomationPoint generatedPoint(double inVal, double outVal,
                                      InterpolationType interp = InterpolationType::Linear)
{
    static uint64_t lastId = 0;

    AutomationPoint p;
    p.inValue = inVal;
    p.outValue = outVal;
    p.interpolation = interp;
    p.itemId = EID::newUniqueTestMode(lastId);

    return p;
}

static AutomationPoint customPoint(double inVal, double outVal,
                                   InterpolationType interp = InterpolationType::Linear)
{
    AutomationPoint p;
    p.inValue = inVal;
    p.outValue = outVal;
    p.interpolation = interp;

    return p;
}

class Automation_Tests : public ::testing::Test
{
};

TEST_F(Automation_Tests, MoveTicks_ShiftsPointsAtAndAfterFrom)
{
    // [GIVEN] Three points on a single curve
    Automation automation;
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    AutomationPoint p1 = generatedPoint(0.3, 0.4);
    AutomationPoint p2 = generatedPoint(0.4, 0.6);
    AutomationPoint p3 = generatedPoint(0.6, 0.7);
    automation.addPoint(key, 100, p1);
    automation.addPoint(key, 200, p2);
    automation.addPoint(key, 300, p3);

    // [WHEN] Move ticks starting at 200 by +100
    automation.moveTicks(200, 100);

    // [THEN] Point before 200 is unchanged; points at 200 and 300 shift to 300 and 400
    AutomationCurve expected;
    expected[100] = p1;
    expected[300] = p2;
    expected[400] = p3;
    checkCurvesMatch(automation.curve(key), expected);
}

TEST_F(Automation_Tests, MoveTicks_NegativeDiff_ShiftsPointsBack)
{
    // [GIVEN] Three points
    Automation automation;
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    AutomationPoint p1 = generatedPoint(0.3, 0.4);
    AutomationPoint p2 = generatedPoint(0.4, 0.6);
    AutomationPoint p3 = generatedPoint(0.6, 0.7);
    automation.addPoint(key, 100, p1);
    automation.addPoint(key, 500, p2);
    automation.addPoint(key, 700, p3);

    // [WHEN] Move ticks starting at 500 by -200
    automation.moveTicks(500, -200);

    // [THEN] Point before 500 is unchanged; 500 -> 300, 700 -> 500
    AutomationCurve expected;
    expected[100] = p1;
    expected[300] = p2;
    expected[500] = p3;
    checkCurvesMatch(automation.curve(key), expected);
}

TEST_F(Automation_Tests, MoveTicks_AcrossMultipleCurves)
{
    // [GIVEN] Two curves with points on different staves
    Automation automation;
    AutomationCurveKey key1;
    key1.type = AutomationType::Dynamics;
    key1.staffId = muse::ID(1);

    AutomationCurveKey key2;
    key2.type = AutomationType::Dynamics;
    key2.staffId = muse::ID(2);

    AutomationPoint p1 = generatedPoint(0.3, 0.5);
    AutomationPoint p2 = generatedPoint(0.4, 0.6);
    automation.addPoint(key1, 200, p1);
    automation.addPoint(key2, 200, p2);

    // [WHEN]
    automation.moveTicks(100, 500);

    // [THEN] Both curves shift from 200 to 700
    AutomationCurve expected1;
    expected1[700] = p1;
    AutomationCurve expected2;
    expected2[700] = p2;
    checkCurvesMatch(automation.curve(key1), expected1);
    checkCurvesMatch(automation.curve(key2), expected2);
}

TEST_F(Automation_Tests, RemoveTicks_RemovesRangeAndClosesGap)
{
    // [GIVEN] Points at 100, 300, 500, 700
    Automation automation;
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    AutomationPoint p100 = generatedPoint(0.3, 0.4);
    AutomationPoint p300 = generatedPoint(0.4, 0.5);
    AutomationPoint p500 = generatedPoint(0.5, 0.6);
    AutomationPoint p700 = generatedPoint(0.6, 0.7);
    automation.addPoint(key, 100, p100);
    automation.addPoint(key, 300, p300);
    automation.addPoint(key, 500, p500);
    automation.addPoint(key, 700, p700);

    // [WHEN] Remove ticks [300, 500] (a 200-tick gap)
    automation.removeTicks(300, 500);

    // [THEN] Points at 300 and 500 removed; 700 shifts to 500
    AutomationCurve expected;
    expected[100] = p100;
    expected[500] = p700;
    checkCurvesMatch(automation.curve(key), expected);
}

TEST_F(Automation_Tests, RemoveTicks_CleansUpEmptyCurves)
{
    // [GIVEN] A curve with all its points inside the removed range
    Automation automation;
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    automation.addPoint(key, 200, generatedPoint(0.4, 0.5));
    automation.addPoint(key, 400, generatedPoint(0.5, 0.6));

    // [WHEN]
    automation.removeTicks(100, 500);

    // [THEN] Curve entry is removed from the map
    EXPECT_TRUE(automation.isEmpty());
}

TEST_F(Automation_Tests, RemovePoints_RemovesGeneratedPreservesCustom)
{
    // [GIVEN] A mix of generated (itemId set) and custom (no itemId) points
    Automation automation;
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    AutomationPoint custom100 = customPoint(0.3, 0.4);
    AutomationPoint custom300 = customPoint(0.5, 0.6);
    automation.addPoint(key, 100, custom100);
    automation.addPoint(key, 200, generatedPoint(0.4, 0.5));
    automation.addPoint(key, 300, custom300);
    automation.addPoint(key, 400, generatedPoint(0.6, 0.7));

    // [WHEN] Remove only generated points
    automation.removePoints([](const AutomationCurveKey&, int, const AutomationPoint& point) {
        return point.itemId.has_value();
    });

    // [THEN] Only custom points remain
    AutomationCurve expected;
    expected[100] = custom100;
    expected[300] = custom300;
    checkCurvesMatch(automation.curve(key), expected);
}

TEST_F(Automation_Tests, RemovePoints_FilterByKeyAndTick)
{
    // [GIVEN] Two curves; remove generated points from key1 at tick >= 300 only
    Automation automation;
    AutomationCurveKey key1;
    key1.type = AutomationType::Dynamics;
    key1.staffId = muse::ID(1);

    AutomationCurveKey key2;
    key2.type = AutomationType::Dynamics;
    key2.staffId = muse::ID(2);

    AutomationPoint p_key1_100 = generatedPoint(0.3, 0.4);
    AutomationPoint p_key2_300 = generatedPoint(0.5, 0.6);
    automation.addPoint(key1, 100, p_key1_100);
    automation.addPoint(key1, 300, generatedPoint(0.4, 0.5));
    automation.addPoint(key2, 300, p_key2_300);

    // [WHEN]
    automation.removePoints([&key1](const AutomationCurveKey& key, int utick, const AutomationPoint& point) {
        return key == key1 && utick >= 300 && point.itemId.has_value();
    });

    // [THEN] key1/300 removed, key1/100 and key2/300 untouched
    AutomationCurve expectedKey1;
    expectedKey1[100] = p_key1_100;
    AutomationCurve expectedKey2;
    expectedKey2[300] = p_key2_300;
    checkCurvesMatch(automation.curve(key1), expectedKey1);
    checkCurvesMatch(automation.curve(key2), expectedKey2);
}

TEST_F(Automation_Tests, RemovePoints_CleansUpEmptyCurves)
{
    // [GIVEN] A curve where all points will be removed by the filter
    Automation automation;
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    automation.addPoint(key, 100, generatedPoint(0.4, 0.5));
    automation.addPoint(key, 200, generatedPoint(0.5, 0.6));

    // [WHEN]
    automation.removePoints([](const AutomationCurveKey&, int, const AutomationPoint& point) {
        return point.itemId.has_value();
    });

    // [THEN]
    EXPECT_TRUE(automation.isEmpty());
}

