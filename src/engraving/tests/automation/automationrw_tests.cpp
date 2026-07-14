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

#include "automation/utils/automationtestutils.h"

using namespace mu::engraving;

class AutomationRW_Tests : public ::testing::Test
{
};

TEST_F(AutomationRW_Tests, RoundTrip_MultipleCurves)
{
    // [GIVEN] Two curves on different staves: key1 holds a generated point (itemId set) plus
    // a SameAsOut and a FromPrevious point; key2 holds a custom point (no itemId)
    Automation automation;
    AutomationCurveKey key1;
    key1.type = AutomationType::Dynamics;
    key1.staffId = muse::ID(1);

    AutomationCurveKey key2;
    key2.type = AutomationType::Dynamics;
    key2.staffId = muse::ID(2);
    key2.voiceIdx = 2;

    const AutomationPoint p1 = generatedPoint(0.3, 0.5);
    const AutomationPoint p2 = customPoint(0.6, 0.8);

    AutomationPoint sameAsOut;
    sameAsOut.inValue = AutomationPoint::SameAsOut {};
    sameAsOut.outValue = 0.5;
    sameAsOut.generated = true;

    AutomationPoint fromPrevious;
    fromPrevious.inValue = AutomationPoint::FromPrevious {};
    fromPrevious.outValue = 0.7;
    fromPrevious.generated = true;

    AutomationCurveMap curves;
    curves[key1] = { { 100, p1 }, { 300, sameAsOut }, { 400, fromPrevious } };
    curves[key2] = { { 200, p2 } };
    automation.setCurves(curves);

    // [WHEN] Serialized (including generated points) and deserialized
    Automation loaded;
    AutomationRW::read(loaded, AutomationRW::write(automation, true /*writeGenerated*/));

    // [THEN] Both curves are preserved with their original points
    checkCurvesMatch(loaded.curve(key1), automation.curve(key1));
    checkCurvesMatch(loaded.curve(key2), automation.curve(key2));

    // [THEN] itemId presence and exact value survive the round trip
    EXPECT_EQ(loaded.curve(key1).at(100).itemId, p1.itemId);
    EXPECT_FALSE(loaded.curve(key2).at(200).itemId.has_value());

    // [THEN] Each point's inValue type survives the round trip
    const AutomationCurve& loadedCurve1 = loaded.curve(key1);
    EXPECT_TRUE(std::holds_alternative<AutomationPoint::SameAsOut>(loadedCurve1.at(300).inValue));
    EXPECT_TRUE(std::holds_alternative<AutomationPoint::FromPrevious>(loadedCurve1.at(400).inValue));
}
