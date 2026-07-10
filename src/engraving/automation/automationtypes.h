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

#include <limits>
#include <optional>
#include <set>
#include <vector>

namespace mu::engraving {
struct AutomationPoint {
    enum class InterpolationType : unsigned char {
        Linear = 0,
        Exponential,
    };

    muse::real_t inValue = 0.; // [0; 1]
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

struct AutomationPointEdit {
    utick_t tick = 0; // tick to write the point at
    AutomationPoint point; // the point's final value
    std::optional<utick_t> moveFrom; // if set, the point currently at this tick is removed as part of this edit
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

    void extend(const AutomationCurveKey& key, utick_t from, utick_t to)
    {
        affectedKeys.insert(key);
        tickFrom = (tickFrom < 0) ? from : std::min(tickFrom, from);
        tickTo = std::max(tickTo, to);
    }

    void clear()
    {
        *this = {};
    }
};
}
