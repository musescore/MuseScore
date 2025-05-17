/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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
#ifndef MU_NOTATION_PIANOKEYBOARDTYPES_H
#define MU_NOTATION_PIANOKEYBOARDTYPES_H

namespace mu::notation {
using piano_key_t = uint8_t;

static constexpr qreal SMALL_KEY_WIDTH_SCALING = 0.5;
static constexpr qreal NORMAL_KEY_WIDTH_SCALING = 1.0;
static constexpr qreal LARGE_KEY_WIDTH_SCALING = 2.0;

enum class KeyState {
    None,
    OtherInSelectedChord,
    Selected,
    Played,
    RightHand,
    LeftHand
};
}

#endif // MU_NOTATION_PIANOKEYBOARDTYPES_H
