/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include <map>
#include <optional>

namespace mu::engraving {
struct AutomationPoint {
    enum class InterpolationType {
        Linear = 0,
        Exponential,
    };

    double inValue = 0.; // [0; 1]
    double outValue = 0.; // [0; 1]
    InterpolationType interpolation = InterpolationType::Linear;
};

enum class AutomationType {
    Unknown = 0,
    Dynamics,
};

using AutomationCurve = std::map<int /*utick*/, AutomationPoint>;

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
}
