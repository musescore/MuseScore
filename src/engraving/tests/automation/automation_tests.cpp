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
#include "global/async/asyncable.h"
#include "global/containers.h"

#include "automation/utils/automationtestutils.h"

using namespace mu::engraving;

class Automation_Tests : public ::testing::Test, public muse::async::Asyncable
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
    AutomationCurveMap curves;
    curves[key] = { { 100, p1 }, { 200, p2 }, { 300, p3 } };
    automation.setCurves(std::move(curves));

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
    AutomationCurveMap curves;
    curves[key] = { { 100, p1 }, { 500, p2 }, { 700, p3 } };
    automation.setCurves(std::move(curves));

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
    AutomationCurveMap curves;
    curves[key1] = { { 200, p1 } };
    curves[key2] = { { 200, p2 } };
    automation.setCurves(std::move(curves));

    // [WHEN] Ticks from 100 shifted by +500
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
    AutomationCurveMap curves;
    curves[key] = { { 100, p100 }, { 300, p300 }, { 500, p500 }, { 700, p700 } };
    automation.setCurves(std::move(curves));

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

    AutomationCurveMap curves;
    curves[key] = { { 200, generatedPoint(0.4, 0.5) }, { 400, generatedPoint(0.5, 0.6) } };
    automation.setCurves(std::move(curves));

    // [WHEN] Remove ticks in range [100, 500]
    automation.removeTicks(100, 500);

    // [THEN] Curve entry is removed from the map
    EXPECT_TRUE(automation.isEmpty());
}

TEST_F(Automation_Tests, RemovePoints_RemovesGivenTicksPreservesOthers)
{
    // [GIVEN] Four points on a single curve
    Automation automation;
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    AutomationPoint custom100 = customPoint(0.3, 0.4);
    AutomationPoint custom300 = customPoint(0.5, 0.6);
    AutomationCurveMap curves;
    curves[key] = {
        { 100, custom100 },
        { 200, generatedPoint(0.4, 0.5) },
        { 300, custom300 },
        { 400, generatedPoint(0.6, 0.7) }
    };
    automation.setCurves(std::move(curves));

    // [WHEN] Remove points at ticks 200 and 400
    automation.removePoints(key, { 200, 400 });

    // [THEN] Only points at 100 and 300 remain
    AutomationCurve expected;
    expected[100] = custom100;
    expected[300] = custom300;
    checkCurvesMatch(automation.curve(key), expected);
}

TEST_F(Automation_Tests, RemovePoints_ScopedToGivenKey)
{
    // [GIVEN] Two curves sharing the same tick
    Automation automation;
    AutomationCurveKey key1;
    key1.type = AutomationType::Dynamics;
    key1.staffId = muse::ID(1);

    AutomationCurveKey key2;
    key2.type = AutomationType::Dynamics;
    key2.staffId = muse::ID(2);

    AutomationPoint p_key1_100 = generatedPoint(0.3, 0.4);
    AutomationPoint p_key2_300 = generatedPoint(0.5, 0.6);
    AutomationCurveMap curves;
    curves[key1] = { { 100, p_key1_100 }, { 300, generatedPoint(0.4, 0.5) } };
    curves[key2] = { { 300, p_key2_300 } };
    automation.setCurves(std::move(curves));

    // [WHEN] Remove tick 300 from key1 only
    automation.removePoints(key1, { 300 });

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
    // [GIVEN] A curve whose only points will all be removed
    Automation automation;
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    AutomationCurveMap curves;
    curves[key] = { { 100, generatedPoint(0.4, 0.5) }, { 200, generatedPoint(0.5, 0.6) } };
    automation.setCurves(std::move(curves));

    // [WHEN] Remove both points
    automation.removePoints(key, { 100, 200 });

    // [THEN] Curve entry is removed from the map
    EXPECT_TRUE(automation.isEmpty());
}

TEST_F(Automation_Tests, ReplaceCurves_MergesReplacesRemovesAndNotifies)
{
    // [GIVEN] Three curves: key1 (to be replaced), key2 (to be emptied/removed), key3 (left untouched)
    Automation automation;
    AutomationCurveKey key1;
    key1.type = AutomationType::Dynamics;
    key1.staffId = muse::ID(1);

    AutomationCurveKey key2;
    key2.type = AutomationType::Dynamics;
    key2.staffId = muse::ID(2);

    AutomationCurveKey key3;
    key3.type = AutomationType::Dynamics;
    key3.staffId = muse::ID(3);

    AutomationPoint p3 = generatedPoint(0.6, 0.7);
    AutomationCurveMap curves;
    curves[key1] = { { 100, generatedPoint(0.3, 0.4) }, { 200, generatedPoint(0.4, 0.5) } };
    curves[key2] = { { 150, generatedPoint(0.5, 0.6) } };
    curves[key3] = { { 250, p3 } };
    automation.setCurves(std::move(curves));

    int notifyCount = 0;
    AutomationChanges lastChanges;
    automation.changed().onReceive(this, [&notifyCount, &lastChanges](const AutomationChanges& ch) {
        ++notifyCount;
        lastChanges = ch;
    });

    // [WHEN] Replace key1 with different points, empty out key2, and don't mention key3
    AutomationPoint newPoint = generatedPoint(0.7, 0.8);
    AutomationCurveMap replacement;
    replacement[key1][300] = newPoint;
    replacement[key2] = AutomationCurve();
    automation.replaceCurves(std::move(replacement));

    // [THEN] key1's old points are gone, only the new one remains
    AutomationCurve expectedKey1;
    expectedKey1[300] = newPoint;
    checkCurvesMatch(automation.curve(key1), expectedKey1);

    // [THEN] key2 is fully removed
    EXPECT_TRUE(automation.curve(key2).empty());

    // [THEN] key3 is untouched
    AutomationCurve expectedKey3;
    expectedKey3[250] = p3;
    checkCurvesMatch(automation.curve(key3), expectedKey3);

    // [THEN] One notification, not a full reset, covering both changed keys and their tick range
    EXPECT_EQ(notifyCount, 1);
    EXPECT_FALSE(lastChanges.isFullReset);
    EXPECT_TRUE(muse::contains(lastChanges.affectedKeys, key1));
    EXPECT_TRUE(muse::contains(lastChanges.affectedKeys, key2));
    EXPECT_FALSE(muse::contains(lastChanges.affectedKeys, key3));
    EXPECT_EQ(lastChanges.tickFrom, 100);
    EXPECT_EQ(lastChanges.tickTo, 300);
}

TEST_F(Automation_Tests, SetCurves_FullyReplacesAndRemovesAbsentKeys)
{
    // [GIVEN] Two curves: key1 (to be replaced), key2 (absent from the new set)
    Automation automation;
    AutomationCurveKey key1;
    key1.type = AutomationType::Dynamics;
    key1.staffId = muse::ID(1);

    AutomationCurveKey key2;
    key2.type = AutomationType::Dynamics;
    key2.staffId = muse::ID(2);

    AutomationCurveMap curves;
    curves[key1] = { { 100, generatedPoint(0.3, 0.4) } };
    curves[key2] = { { 150, generatedPoint(0.5, 0.6) } };
    automation.replaceCurves(std::move(curves));

    int notifyCount = 0;
    AutomationChanges lastChanges;
    automation.changed().onReceive(this, [&notifyCount, &lastChanges](const AutomationChanges& ch) {
        ++notifyCount;
        lastChanges = ch;
    });

    // [WHEN] Setting curves that only mention key1
    AutomationPoint newPoint = generatedPoint(0.7, 0.8);
    AutomationCurveMap newCurves;
    newCurves[key1][300] = newPoint;
    automation.setCurves(std::move(newCurves));

    // [THEN] key1 has only the new point
    AutomationCurve expectedKey1;
    expectedKey1[300] = newPoint;
    checkCurvesMatch(automation.curve(key1), expectedKey1);

    // [THEN] key2 is gone, it wasn't mentioned in the new curve set
    EXPECT_TRUE(automation.curve(key2).empty());

    // [THEN] One notification, reported as a full reset
    EXPECT_EQ(notifyCount, 1);
    EXPECT_TRUE(lastChanges.isFullReset);
}

TEST_F(Automation_Tests, SetCurves_SameContent_NotFired)
{
    // [GIVEN] A curve already matching what's about to be set
    Automation automation;
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    AutomationCurveMap curves;
    curves[key] = { { 100, generatedPoint(0.3, 0.4) } };
    automation.replaceCurves(AutomationCurveMap(curves));

    int notifyCount = 0;
    automation.changed().onReceive(this, [&notifyCount](const AutomationChanges&) {
        ++notifyCount;
    });

    // [WHEN] Setting identical curves
    automation.setCurves(std::move(curves));

    // [THEN] No notification fired
    EXPECT_EQ(notifyCount, 0);
}

TEST_F(Automation_Tests, Notify_EditPoints_FiresWithCorrectRange)
{
    // [GIVEN] A subscriber registered before the change
    Automation automation;
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    int notifyCount = 0;
    AutomationChanges lastChanges;
    automation.changed().onReceive(this, [&notifyCount, &lastChanges](const AutomationChanges& ch) {
        ++notifyCount;
        lastChanges = ch;
    });

    // [WHEN] Add a point
    automation.editPoints(key, { { 200, generatedPoint(0.5, 0.5) } });

    // [THEN] Exactly one notification with the correct range
    EXPECT_EQ(notifyCount, 1);
    EXPECT_FALSE(lastChanges.isFullReset);
    EXPECT_TRUE(muse::contains(lastChanges.affectedKeys, key));
    EXPECT_EQ(lastChanges.tickFrom, 200);
    EXPECT_EQ(lastChanges.tickTo, 200);
}

TEST_F(Automation_Tests, Notify_EditPoints_SameValue_NotFired)
{
    // [GIVEN] A point already at 0.5
    Automation automation;
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    const AutomationPoint point = generatedPoint(0.5, 0.5);
    AutomationCurveMap curves;
    curves[key] = { { 100, point } };
    automation.setCurves(std::move(curves));

    int notifyCount = 0;
    automation.changed().onReceive(this, [&notifyCount](const AutomationChanges&) {
        ++notifyCount;
    });

    // [WHEN] Set an identical point
    automation.editPoints(key, { { 100, point } });

    // [THEN] No notification fired
    EXPECT_EQ(notifyCount, 0);
}

TEST_F(Automation_Tests, Notify_MoveTicks_OpenEndedRange)
{
    // [GIVEN] Two points and a subscriber
    Automation automation;
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    AutomationCurveMap curves;
    curves[key] = { { 100, generatedPoint(0.3, 0.4) }, { 200, generatedPoint(0.5, 0.6) } };
    automation.setCurves(std::move(curves));

    AutomationChanges lastChanges;
    automation.changed().onReceive(this, [&lastChanges](const AutomationChanges& ch) {
        lastChanges = ch;
    });

    // [WHEN] Ticks shifted from 100 by +50
    automation.moveTicks(100, 50);

    // [THEN] tickTo is open-ended (all points from tickFrom forward are affected)
    EXPECT_EQ(lastChanges.tickFrom, 100);
    EXPECT_EQ(lastChanges.tickTo, std::numeric_limits<utick_t>::max());
}
