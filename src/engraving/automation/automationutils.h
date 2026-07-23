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

#include "automationtypes.h"

#include <algorithm>
#include <type_traits>

namespace mu::engraving {
//! NOTE: resolves what a point's arrival value actually is, given where it sits in its curve
inline muse::real_t resolvedInValue(const AutomationCurve& curve, AutomationCurve::const_iterator it)
{
    return std::visit([&](const auto& v) -> muse::real_t {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, AutomationPoint::FromPrevious>) {
            return it == curve.begin() ? muse::real_t(0.0) : std::prev(it)->second.outValue;
        } else if constexpr (std::is_same_v<T, AutomationPoint::SameAsOut>) {
            return it->second.outValue;
        } else {
            return v;
        }
    }, it->second.inValue);
}

//! NOTE: Value at normalized position t ([0, 1]) across the segment arriving at it
//! The segment is split into two quadratic Bezier arcs meeting at the bend
//! point. The arcs share a tangent there, producing a smooth bend whose position
//! follows bend.t. When isNone(), this reduces to a straight line
inline muse::real_t evaluateCurveAt(const AutomationCurve& curve, AutomationCurve::const_iterator it, muse::real_t t)
{
    const muse::real_t prevOut = it == curve.begin() ? muse::real_t(0.0) : std::prev(it)->second.outValue;
    const muse::real_t thisIn = resolvedInValue(curve, it);

    const AutomationPoint::Bend& bend = it->second.bend;
    const muse::real_t bendT = bend.t;

    if (bendT <= 0.0 || bendT >= 1.0) {
        return prevOut + (thisIn - prevOut) * t;
    }

    const muse::real_t fraction = std::clamp(bend.value, muse::real_t(0.0), muse::real_t(1.0));
    const muse::real_t bendValue = prevOut + fraction * (thisIn - prevOut);

    const muse::real_t lo = std::min(prevOut, thisIn);
    const muse::real_t hi = std::max(prevOut, thisIn);
    const muse::real_t halfSlope = 0.5 * (thisIn - prevOut);

    // Clamped control points for each arc
    const muse::real_t q1 = std::clamp(bendValue - bendT * halfSlope, lo, hi);
    const muse::real_t q2 = std::clamp(bendValue + (1.0 - bendT) * halfSlope, lo, hi);

    if (t <= bendT) {
        const muse::real_t s = t / bendT;
        const muse::real_t u = 1.0 - s;
        return u * u * prevOut + 2.0 * u * s * q1 + s * s * bendValue;
    }

    const muse::real_t s = (t - bendT) / (1.0 - bendT);
    const muse::real_t u = 1.0 - s;
    return u * u * bendValue + 2.0 * u * s * q2 + s * s * thisIn;
}
}
