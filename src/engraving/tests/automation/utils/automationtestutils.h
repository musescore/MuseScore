/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
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
#pragma once

#include <gtest/gtest.h>

#include "engraving/automation/automationtypes.h"

namespace mu::engraving {
using AutomationEase = AutomationPoint::Ease;

inline AutomationPoint generatedPoint(double inVal, double outVal, AutomationEase ease = AutomationEase::none())
{
    static uint64_t lastId = 0;

    AutomationPoint p;
    p.value.outValue = outVal;
    p.value.inValue = AutomationPoint::ExplicitArrival { muse::real_t(inVal), ease };
    p.itemId = EID::newUniqueTestMode(lastId);
    p.generated = true;

    return p;
}

inline AutomationPoint customPoint(double inVal, double outVal, AutomationEase ease = AutomationEase::none())
{
    AutomationPoint p;
    p.value.outValue = outVal;
    p.value.inValue = AutomationPoint::ExplicitArrival { muse::real_t(inVal), ease };
    p.generated = false;

    return p;
}

inline void checkCurvesMatch(const AutomationCurve& actualCurve, const AutomationCurve& expectedCurve)
{
    EXPECT_EQ(actualCurve.size(), expectedCurve.size());

    for (auto expectedIt = expectedCurve.cbegin(); expectedIt != expectedCurve.cend(); ++expectedIt) {
        const utick_t tick = expectedIt->first;
        const auto actualIt = actualCurve.find(tick);
        ASSERT_TRUE(actualIt != actualCurve.cend()) << "Missing point at tick " << tick;
        const AutomationPoint& actualPoint = actualIt->second;
        const AutomationPoint& expectedPoint = expectedIt->second;

        EXPECT_NEAR(resolveInValue(actualCurve, actualIt), resolveInValue(expectedCurve, expectedIt), 0.0001)
            << "inValue mismatch at tick " << tick;
        EXPECT_NEAR(actualPoint.value.outValue, expectedPoint.value.outValue, 0.0001) << "outValue mismatch at tick " << tick;
        EXPECT_EQ(ease(actualPoint).value_or(AutomationEase::none()), ease(expectedPoint).value_or(AutomationEase::none()))
            << "ease mismatch at tick " << tick;
        EXPECT_EQ(actualPoint.generated, expectedPoint.generated) << "generated mismatch at ticK " << tick;
    }
}
}
