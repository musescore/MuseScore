/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_INSPECTOR_BARLINETYPES_H
#define MU_INSPECTOR_BARLINETYPES_H

#include "qobjectdefs.h"

namespace mu::inspector {
class BarlineTypes
{
    Q_GADGET

public:
    enum class LineType {
        TYPE_NORMAL = 1,
        TYPE_SINGLE= LineType::TYPE_NORMAL,
        TYPE_DOUBLE = 2,
        TYPE_START_REPEAT = 4,
        TYPE_LEFT_REPEAT = LineType::TYPE_START_REPEAT,
        TYPE_END_REPEAT = 8,
        TYPE_RIGHT_REPEAT = LineType::TYPE_END_REPEAT,
        TYPE_DASHED = 0x10,
        TYPE_BROKEN = LineType::TYPE_DASHED,
        TYPE_FINAL = 0x20,
        TYPE_END = LineType::TYPE_FINAL,
        TYPE_END_START_REPEAT = 0x40,
        TYPE_LEFT_RIGHT_REPEAT = LineType::TYPE_END_START_REPEAT,
        TYPE_DOTTED = 0x80,
        TYPE_REVERSE_END = 0x100,
        TYPE_REVERS_FINAL = LineType::TYPE_REVERSE_END,
        TYPE_HEAVY = 0x200,
        TYPE_DOUBLE_HEAVY = 0x400,
    };

    enum class SpanPreset {
        PRESET_DEFAULT,
        PRESET_TICK_1,
        PRESET_TICK_2,
        PRESET_SHORT_1,
        PRESET_SHORT_2,
    };

    Q_ENUM(LineType)
    Q_ENUM(SpanPreset)
};
}

#endif // MU_INSPECTOR_BARLINETYPES_H
