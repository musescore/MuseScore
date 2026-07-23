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

#include "engraving/automation/automationutils.h"

namespace mu::engraving {
using AutomationBend = AutomationPoint::Bend;

inline AutomationPoint generatedPoint(double inVal, double outVal, AutomationBend bend = AutomationBend::none())
{
    static uint64_t lastId = 0;

    AutomationPoint p;
    p.inValue = inVal;
    p.outValue = outVal;
    p.bend = bend;
    p.itemId = EID::newUniqueTestMode(lastId);
    p.generated = true;

    return p;
}

inline AutomationPoint customPoint(double inVal, double outVal, AutomationBend bend = AutomationBend::none())
{
    AutomationPoint p;
    p.inValue = inVal;
    p.outValue = outVal;
    p.bend = bend;
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

        EXPECT_NEAR(resolvedInValue(actualCurve, actualIt), resolvedInValue(expectedCurve, expectedIt), 0.0001)
            << "inValue mismatch at tick " << tick;
        EXPECT_NEAR(actualPoint.outValue, expectedPoint.outValue, 0.0001) << "outValue mismatch at tick " << tick;
        EXPECT_EQ(actualPoint.bend, expectedPoint.bend) << "bend mismatch at tick " << tick;
        EXPECT_EQ(actualPoint.generated, expectedPoint.generated) << "generated mismatch at ticK " << tick;
    }
}
}
