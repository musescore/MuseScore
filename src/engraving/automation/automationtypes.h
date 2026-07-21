/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
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
#pragma once

#include "global/types/id.h"
#include "global/types/number.h"
#include "global/types/sharedmap.h"

#include "engraving/infrastructure/eid.h"
#include "engraving/types/types.h"

#include <limits>
#include <optional>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

namespace mu::engraving {
// Normalized [0.0, 1.0] dynamic levels, aligned with MPE dynamic level percentages (5% steps starting at 17.5%)
inline const std::unordered_map<DynamicType, real_t> ORDINARY_DYNAMIC_VALUES {
    { DynamicType::N,      0.000 },
    { DynamicType::PPPPPP, 0.175 },
    { DynamicType::PPPPP,  0.225 },
    { DynamicType::PPPP,   0.275 },
    { DynamicType::PPP,    0.325 },
    { DynamicType::PP,     0.375 },
    { DynamicType::P,      0.425 },
    { DynamicType::MP,     0.475 },
    { DynamicType::MF,     0.525 },
    { DynamicType::F,      0.575 },
    { DynamicType::FF,     0.625 },
    { DynamicType::FFF,    0.675 },
    { DynamicType::FFFF,   0.725 },
    { DynamicType::FFFFF,  0.775 },
    { DynamicType::FFFFFF, 0.825 },
};

struct AutomationPoint {
    enum class InterpolationType : unsigned char {
        Linear = 0,
        Exponential,
    };

    //! NOTE: arrival value equals whatever precedes this point in the curve, resolved live
    struct FromPrevious {
        bool operator==(const FromPrevious&) const { return true; }
    };
    //! NOTE: arrival value equals this point's own outValue (a flat, held point)
    struct SameAsOut {
        bool operator==(const SameAsOut&) const { return true; }
    };
    using InValue = std::variant<FromPrevious, SameAsOut, muse::real_t>;

    InValue inValue = FromPrevious {};
    muse::real_t outValue = 0.; // [0; 1]
    InterpolationType interpolation = InterpolationType::Linear;
    std::optional<EID> itemId; // valid if it was created from an engraving item (e.g., Dynamic)
    bool generated = false; // true if the point was generated automatically and hasn't been edited by the user

    bool operator==(const AutomationPoint& p) const
    {
        return inValue == p.inValue
               && outValue == p.outValue
               && interpolation == p.interpolation
               && itemId == p.itemId
               && generated == p.generated;
    }
};

enum class AutomationType : unsigned char {
    Unknown = 0,
    Dynamics,
};

struct AutomationCurveKey {
    AutomationType type = AutomationType::Unknown;
    muse::ID staffId;
    std::optional<size_t> voiceIdx;

    bool isValid() const
    {
        return type != AutomationType::Unknown;
    }

    bool operator==(const AutomationCurveKey& k) const
    {
        return type == k.type && staffId == k.staffId && voiceIdx == k.voiceIdx;
    }

    bool operator<(const AutomationCurveKey& k) const
    {
        return std::tie(type, staffId, voiceIdx) < std::tie(k.type, k.staffId, k.voiceIdx);
    }
};

using utick_t = int;
using AutomationCurve = muse::SharedMap<utick_t, AutomationPoint>;
using AutomationCurveMap = muse::SharedMap<AutomationCurveKey, AutomationCurve>;

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

struct AutomationPointEdit {
    //! NOTE: write point at tick
    struct SetPoint {
        AutomationPoint point;
    };
    //! NOTE: write point at tick, removing whatever point currently sits at from
    struct MovePoint {
        AutomationPoint point;
        utick_t from = 0;
    };
    //! NOTE: erase whatever point currently sits at tick
    struct ErasePoint {};

    utick_t tick = 0; // destination tick for SetPoint/MovePoint, or the tick to erase for ErasePoint
    std::variant<SetPoint, MovePoint, ErasePoint> change;
};

using AutomationPointEdits = std::vector<AutomationPointEdit>;

struct AutomationChanges {
    bool isFullReset = false;
    std::set<AutomationCurveKey> affectedKeys;
    utick_t tickFrom = -1;
    utick_t tickTo = -1;

    bool isEmpty() const
    {
        return !isFullReset && affectedKeys.empty();
    }

    void extend(utick_t from, utick_t to)
    {
        tickFrom = (tickFrom < 0) ? from : std::min(tickFrom, from);
        tickTo = std::max(tickTo, to);
    }

    void extend(const AutomationCurveKey& key, utick_t from, utick_t to)
    {
        affectedKeys.insert(key);
        extend(from, to);
    }

    void clear()
    {
        *this = {};
    }
};
}
