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

#include "engraving/automation/automationdata.h"
#include "global/async/asyncable.h"
#include "global/containers.h"

#include "automation/utils/automationtestutils.h"

using namespace mu::engraving;

class Automation_Tests : public ::testing::Test, public muse::async::Asyncable
{
};

TEST_F(Automation_Tests, EditPoints_EraseRemovesGivenTicksPreservesOthers)
{
    // [GIVEN] Four points on a single curve
    AutomationData data;
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
    data.setCurves(curves);

    // [WHEN] Erase points at ticks 200 and 400
    data.editPoints(key, {
        { 200, AutomationPointEdit::ErasePoint {} },
        { 400, AutomationPointEdit::ErasePoint {} }
    });

    // [THEN] Only points at 100 and 300 remain
    AutomationCurve expected;
    expected[100] = custom100;
    expected[300] = custom300;
    checkCurvesMatch(data.curve(key), expected);
}

TEST_F(Automation_Tests, EditPoints_EraseScopedToGivenKey)
{
    // [GIVEN] Two curves sharing the same tick
    AutomationData data;
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
    data.setCurves(curves);

    // [WHEN] Erase tick 300 from key1 only
    data.editPoints(key1, { { 300, AutomationPointEdit::ErasePoint {} } });

    // [THEN] key1/300 removed, key1/100 and key2/300 untouched
    AutomationCurve expectedKey1;
    expectedKey1[100] = p_key1_100;
    AutomationCurve expectedKey2;
    expectedKey2[300] = p_key2_300;
    checkCurvesMatch(data.curve(key1), expectedKey1);
    checkCurvesMatch(data.curve(key2), expectedKey2);
}

TEST_F(Automation_Tests, EditPoints_EraseCleansUpEmptyCurves)
{
    // [GIVEN] A curve whose only points will all be erased
    AutomationData data;
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    AutomationCurveMap curves;
    curves[key] = { { 100, generatedPoint(0.4, 0.5) }, { 200, generatedPoint(0.5, 0.6) } };
    data.setCurves(curves);

    // [WHEN] Erase both points
    data.editPoints(key, {
        { 100, AutomationPointEdit::ErasePoint {} },
        { 200, AutomationPointEdit::ErasePoint {} }
    });

    // [THEN] Curve entry is removed from the map
    EXPECT_TRUE(data.isEmpty());
}

TEST_F(Automation_Tests, ReplaceCurves_MergesReplacesRemovesAndNotifies)
{
    // [GIVEN] Three curves: key1 (to be replaced), key2 (to be emptied/removed), key3 (left untouched)
    AutomationData data;
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
    data.setCurves(curves);

    int notifyCount = 0;
    AutomationChanges lastChanges;
    data.changed().onReceive(this, [&notifyCount, &lastChanges](const AutomationChanges& ch) {
        ++notifyCount;
        lastChanges = ch;
    });

    // [WHEN] Replace key1 with different points, empty out key2, and don't mention key3
    AutomationPoint newPoint = generatedPoint(0.7, 0.8);
    AutomationCurveMap replacement;
    replacement[key1][300] = newPoint;
    replacement[key2] = AutomationCurve();
    data.replaceCurves(replacement);

    // [THEN] key1's old points are gone, only the new one remains
    AutomationCurve expectedKey1;
    expectedKey1[300] = newPoint;
    checkCurvesMatch(data.curve(key1), expectedKey1);

    // [THEN] key2 is fully removed
    EXPECT_TRUE(data.curve(key2).empty());

    // [THEN] key3 is untouched
    AutomationCurve expectedKey3;
    expectedKey3[250] = p3;
    checkCurvesMatch(data.curve(key3), expectedKey3);

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
    AutomationData data;
    AutomationCurveKey key1;
    key1.type = AutomationType::Dynamics;
    key1.staffId = muse::ID(1);

    AutomationCurveKey key2;
    key2.type = AutomationType::Dynamics;
    key2.staffId = muse::ID(2);

    AutomationCurveMap curves;
    curves[key1] = { { 100, generatedPoint(0.3, 0.4) } };
    curves[key2] = { { 150, generatedPoint(0.5, 0.6) } };
    data.replaceCurves(curves);

    int notifyCount = 0;
    AutomationChanges lastChanges;
    data.changed().onReceive(this, [&notifyCount, &lastChanges](const AutomationChanges& ch) {
        ++notifyCount;
        lastChanges = ch;
    });

    // [WHEN] Setting curves that only mention key1
    AutomationPoint newPoint = generatedPoint(0.7, 0.8);
    AutomationCurveMap newCurves;
    newCurves[key1][300] = newPoint;
    data.setCurves(newCurves);

    // [THEN] key1 has only the new point
    AutomationCurve expectedKey1;
    expectedKey1[300] = newPoint;
    checkCurvesMatch(data.curve(key1), expectedKey1);

    // [THEN] key2 is gone, it wasn't mentioned in the new curve set
    EXPECT_TRUE(data.curve(key2).empty());

    // [THEN] One notification, reported as a full reset
    EXPECT_EQ(notifyCount, 1);
    EXPECT_TRUE(lastChanges.isFullReset);
}

TEST_F(Automation_Tests, SetCurves_SameContent_NotFired)
{
    // [GIVEN] A curve already matching what's about to be set
    AutomationData data;
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    AutomationCurveMap curves;
    curves[key] = { { 100, generatedPoint(0.3, 0.4) } };
    data.replaceCurves(curves);

    int notifyCount = 0;
    data.changed().onReceive(this, [&notifyCount](const AutomationChanges&) {
        ++notifyCount;
    });

    // [WHEN] Setting identical curves
    data.setCurves(curves);

    // [THEN] No notification fired
    EXPECT_EQ(notifyCount, 0);
}

TEST_F(Automation_Tests, Notify_EditPoints_FiresWithCorrectRange)
{
    // [GIVEN] A subscriber registered before the change
    AutomationData data;
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    int notifyCount = 0;
    AutomationChanges lastChanges;
    data.changed().onReceive(this, [&notifyCount, &lastChanges](const AutomationChanges& ch) {
        ++notifyCount;
        lastChanges = ch;
    });

    // [WHEN] Add a point
    data.editPoints(key, { { 200, AutomationPointEdit::SetPoint { generatedPoint(0.5, 0.5) } } });

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
    AutomationData data;
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    const AutomationPoint point = generatedPoint(0.5, 0.5);
    AutomationCurveMap curves;
    curves[key] = { { 100, point } };
    data.setCurves(curves);

    int notifyCount = 0;
    data.changed().onReceive(this, [&notifyCount](const AutomationChanges&) {
        ++notifyCount;
    });

    // [WHEN] Set an identical point
    data.editPoints(key, { { 100, AutomationPointEdit::SetPoint { point } } });

    // [THEN] No notification fired
    EXPECT_EQ(notifyCount, 0);
}
