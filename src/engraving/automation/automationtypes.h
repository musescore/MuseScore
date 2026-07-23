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

#include <algorithm>
#include <optional>
#include <set>
#include <unordered_map>
#include <variant>
#include <vector>

namespace mu::engraving {
struct AutomationPoint {
    //! NOTE: arrival value equals whatever precedes this point in the curve, resolved live
    struct FromPrevious {
        bool operator==(const FromPrevious&) const { return true; }
    };
    //! NOTE: arrival value equals this point's own outValue (a flat, held point)
    struct SameAsOut {
        bool operator==(const SameAsOut&) const { return true; }
    };
    using InValue = std::variant<FromPrevious, SameAsOut, muse::real_t>;

    //! NOTE: bends the segment through point (t, value)
    struct Bend {
        muse::real_t t = 0.5; // [0; 1]
        muse::real_t value = 0.5; // [0; 1]

        static Bend none() { return {}; }
        bool isNone() const { return *this == none(); }
        bool operator==(const Bend& b) const { return t == b.t && value == b.value; }
    };

    InValue inValue = FromPrevious {};
    muse::real_t outValue = 0.; // [0; 1]
    Bend bend;
    std::optional<EID> itemId; // valid if it was created from an engraving item (e.g., Dynamic)
    bool generated = false; // true if the point was generated automatically and hasn't been edited by the user

    bool operator==(const AutomationPoint& p) const
    {
        return inValue == p.inValue
               && outValue == p.outValue
               && bend == p.bend
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
