/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) MuseScore Limited and others
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

#include "rcommand/commandtypes.h"

namespace mu::palette {
inline static const muse::rcommand::Command TOGGLE_MASTER_PALETTE_COMMAND("command://palette/toggle-master-palette");
inline static const muse::rcommand::Command TOGGLE_SPECIAL_CHARACTERS_COMMAND("command://palette/toggle-special-characters");
inline static const muse::rcommand::Command OPEN_TIME_SIGNATURE_PROPERTIES_COMMAND("command://palette/open-time-signature-properties");
inline static const muse::rcommand::Command OPEN_CUSTOMIZE_KIT_COMMAND("command://palette/open-customize-kit");

inline static const muse::rcommand::Command PALETTE_SEARCH_COMMAND("command://palette/search");
inline static const muse::rcommand::Command PALETTE_APPLY_CURRENT_ELEMENT_COMMAND("command://palette/apply-current-element");

inline static const muse::rcommand::Command PALETTE_TOGGLE_SINGLE_CLICK_TO_OPEN_COMMAND("command://palette/toggle-single-click-to-open");
inline static const muse::rcommand::Command PALETTE_TOGGLE_SINGLE_PALETTE_COMMAND("command://palette/toggle-single-palette");
inline static const muse::rcommand::Command PALETTE_TOGGLE_DRAG_ENABLED_COMMAND("command://palette/toggle-drag-enabled");
inline static const muse::rcommand::Command PALETTE_EXPAND_ALL_COMMAND("command://palette/expand-all");
inline static const muse::rcommand::Command PALETTE_COLLAPSE_ALL_COMMAND("command://palette/collapse-all");
}
