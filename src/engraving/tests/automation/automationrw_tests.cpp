/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#include "engraving/automation/internal/automationrw.h"

using namespace mu::engraving;

static void checkCurvesMatch(const AutomationCurve& actualCurve, const AutomationCurve& expectedCurve)
{
    EXPECT_EQ(actualCurve.size(), expectedCurve.size());

    for (const auto& [tick, expectedPoint] : expectedCurve) {
        ASSERT_TRUE(actualCurve.count(tick)) << "Missing point at tick " << tick;
        const AutomationPoint& actualPoint = actualCurve.at(tick);

        EXPECT_NEAR(actualPoint.inValue, expectedPoint.inValue, 0.0001) << "inValue mismatch at tick " << tick;
        EXPECT_NEAR(actualPoint.outValue, expectedPoint.outValue, 0.0001) << "outValue mismatch at tick " << tick;
        EXPECT_EQ(actualPoint.interpolation, expectedPoint.interpolation) << "interpolation mismatch at tick " << tick;
    }
}

class AutomationRW_Tests : public ::testing::Test
{
};

TEST_F(AutomationRW_Tests, RoundTrip_MultipleCurves)
{
    // [GIVEN] Two curves on different staves
    Automation automation;
    AutomationCurveKey key1;
    key1.type = AutomationType::Dynamics;
    key1.staffId = muse::ID(1);

    AutomationCurveKey key2;
    key2.type = AutomationType::Dynamics;
    key2.staffId = muse::ID(2);
    key2.voiceIdx = 2;

    AutomationPoint p1;
    p1.inValue = 0.3;
    p1.outValue = 0.5;
    AutomationPoint p2;
    p2.inValue = 0.6;
    p2.outValue = 0.8;
    automation.addPoint(key1, 100, p1);
    automation.addPoint(key2, 200, p2);

    // [WHEN] Serialized and deserialized
    Automation loaded;
    AutomationRW::read(loaded, AutomationRW::write(automation, true /*writeGenerated*/));

    // [THEN] Both curves are preserved with their original points
    checkCurvesMatch(loaded.curve(key1), automation.curve(key1));
    checkCurvesMatch(loaded.curve(key2), automation.curve(key2));
}
