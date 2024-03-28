/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_WORKSPACE_WORKSPACETYPES_H
#define MU_WORKSPACE_WORKSPACETYPES_H

namespace mu::workspace {
static const std::string DEFAULT_WORKSPACE_NAME("Default");

using DataKey = const char*;
inline constexpr DataKey WS_Undefined("");

// inline std::string key_to_string(DataKey key)
// {
//     switch (key) {
//     case DataKey::Undefined: return std::string();
//     case DataKey::UiSettings: return std::string("ui_settings");
//     case DataKey::UiStates: return std::string("ui_states");
//     case DataKey::UiToolConfigs: return std::string("ui_toolconfigs");
//     case DataKey::Palettes: return std::string("palettes");
//     }
//     return std::string();
// }
}

#endif // MU_WORKSPACE_WORKSPACETYPES_H
