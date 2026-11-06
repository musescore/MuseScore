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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <optional>
#include <string_view>
#include <utility>

#include "engraving/automation/automationtypes.h"

namespace mu::notation {
enum class AutomationLane : unsigned char {
    Dynamics = 0,
    Tempo,
    Volume,
    Pan,
    MasterVolume,
    MasterPan,
};

inline constexpr std::pair<AutomationLane, std::string_view> AUTOMATION_LANE_NAMES[] = {
    { AutomationLane::Dynamics, "dynamics" },
    { AutomationLane::Tempo, "tempo" },
    { AutomationLane::Volume, "volume" },
    { AutomationLane::Pan, "pan" },
    { AutomationLane::MasterVolume, "master-volume" },
    { AutomationLane::MasterPan, "master-pan" },
};

inline std::string_view automationLaneToString(AutomationLane lane)
{
    for (const auto& [l, name] : AUTOMATION_LANE_NAMES) {
        if (l == lane) {
            return name;
        }
    }

    return {};
}

inline std::optional<AutomationLane> automationLaneFromString(std::string_view str)
{
    for (const auto& [lane, name] : AUTOMATION_LANE_NAMES) {
        if (name == str) {
            return lane;
        }
    }

    return std::nullopt;
}

inline engraving::AutomationType automationTypeOf(AutomationLane lane)
{
    switch (lane) {
    case AutomationLane::Dynamics: return engraving::AutomationType::Dynamics;
    case AutomationLane::Tempo: return engraving::AutomationType::Tempo;
    case AutomationLane::Volume:
    case AutomationLane::MasterVolume: return engraving::AutomationType::Volume;
    case AutomationLane::Pan:
    case AutomationLane::MasterPan: return engraving::AutomationType::Pan;
    }

    return engraving::AutomationType::Unknown;
}
}
