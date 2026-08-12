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

#include "engraving/automation/automationdata.h"
#include "engraving/automation/internal/automationrw.h"

#include "automation/utils/automationtestutils.h"

using namespace mu::engraving;

class AutomationRW_Tests : public ::testing::Test
{
};

TEST_F(AutomationRW_Tests, RoundTrip_StaffScope)
{
    // [GIVEN] Two staff-scoped curves: key1 holds a generated point (itemId set) plus
    // an explicit-arrival and a FromPrevious point; key2 holds a custom point (no itemId)
    AutomationData data;
    AutomationCurveKey key1 = AutomationCurveKey::staff(AutomationType::Dynamics, muse::ID(1));
    AutomationCurveKey key2 = AutomationCurveKey::staff(AutomationType::Dynamics, muse::ID(2), size_t(2));

    const AutomationPoint p1 = generatedPoint(0.3, 0.5);
    const AutomationPoint p2 = customPoint(0.6, 0.8);

    AutomationPoint explicitArrival;
    explicitArrival.value.outValue = 0.5;
    explicitArrival.value.inValue = AutomationPoint::ExplicitArrival { explicitArrival.value.outValue, AutomationPoint::Ease::none() };
    explicitArrival.generated = true;

    AutomationPoint fromPrevious;
    fromPrevious.value.inValue = AutomationPoint::ArrivalFromPrevious {};
    fromPrevious.value.outValue = 0.7;
    fromPrevious.generated = true;

    AutomationCurveMap curves;
    curves[key1] = { { 100, p1 }, { 300, explicitArrival }, { 400, fromPrevious } };
    curves[key2] = { { 200, p2 } };
    data.setCurves(curves);

    // [WHEN] Serialized (including generated points) and deserialized
    AutomationData loaded;
    AutomationRW::read(loaded, AutomationRW::write(data, true /*writeGenerated*/));

    // [THEN] Both curves are preserved with their original points
    checkCurvesMatch(loaded.curve(key1), data.curve(key1));
    checkCurvesMatch(loaded.curve(key2), data.curve(key2));

    // [THEN] itemId presence and exact value survive the round trip
    EXPECT_EQ(loaded.curve(key1).at(100).itemId, p1.itemId);
    EXPECT_FALSE(loaded.curve(key2).at(200).itemId.has_value());

    // [THEN] Each point's inValue type survives the round trip
    const AutomationCurve& loadedCurve1 = loaded.curve(key1);
    EXPECT_TRUE(std::holds_alternative<AutomationPoint::ExplicitArrival>(loadedCurve1.at(300).value.inValue));
    EXPECT_TRUE(std::holds_alternative<AutomationPoint::ArrivalFromPrevious>(loadedCurve1.at(400).value.inValue));
}

TEST_F(AutomationRW_Tests, RoundTrip_InstrumentScope)
{
    // [GIVEN] An instrument-scoped curve
    // (e.g. Pan, owned by a specific part+instrument, not any particular staff)
    AutomationData data;

    InstrumentTrackId trackId;
    trackId.partId = muse::ID(3);
    trackId.instrumentId = u"instrument1";
    AutomationCurveKey key = AutomationCurveKey::instrument(AutomationType::Pan, trackId);

    const AutomationPoint p = customPoint(0.7, 0.9);

    AutomationCurveMap curves;
    curves[key] = { { 600, p } };
    data.setCurves(curves);

    // [WHEN] Serialized and deserialized
    AutomationData loaded;
    AutomationRW::read(loaded, AutomationRW::write(data, true /*writeGenerated*/));

    // [THEN] The curve is preserved with its original points
    checkCurvesMatch(loaded.curve(key), data.curve(key));

    // [THEN] The loaded key is exactly equal to the original
    ASSERT_EQ(loaded.curves().size(), 1u);

    const auto loadedKeyIt = loaded.curves().find(key);
    ASSERT_NE(loadedKeyIt, loaded.curves().end());
    EXPECT_EQ(loadedKeyIt->first, key);
    EXPECT_EQ(loadedKeyIt->first.type, AutomationType::Pan);
    ASSERT_TRUE(loadedKeyIt->first.trackId().has_value());
    EXPECT_EQ(loadedKeyIt->first.trackId()->partId, trackId.partId);
    EXPECT_EQ(loadedKeyIt->first.trackId()->instrumentId, trackId.instrumentId);
}

TEST_F(AutomationRW_Tests, RoundTrip_GlobalScope)
{
    // [GIVEN] A global-scoped curve (e.g. Volume with no owning staff/instrument)
    AutomationData data;
    AutomationCurveKey key = AutomationCurveKey::global(AutomationType::Volume);

    const AutomationPoint p = customPoint(0.2, 0.4);

    AutomationCurveMap curves;
    curves[key] = { { 500, p } };
    data.setCurves(curves);

    // [WHEN] Serialized and deserialized
    AutomationData loaded;
    AutomationRW::read(loaded, AutomationRW::write(data, true /*writeGenerated*/));

    // [THEN] The curve is preserved with its original points
    checkCurvesMatch(loaded.curve(key), data.curve(key));

    // [THEN] The loaded key is exactly equal to the original
    ASSERT_EQ(loaded.curves().size(), 1u);

    const auto loadedKeyIt = loaded.curves().find(key);
    ASSERT_NE(loadedKeyIt, loaded.curves().end());
    EXPECT_EQ(loadedKeyIt->first, key);
    EXPECT_EQ(loadedKeyIt->first.type, AutomationType::Volume);
    EXPECT_FALSE(loadedKeyIt->first.trackId().has_value());
    EXPECT_FALSE(loadedKeyIt->first.staffId().has_value());
}
