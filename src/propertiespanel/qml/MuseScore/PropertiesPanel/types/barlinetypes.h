/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <qqmlintegration.h>

#include "engraving/types/types.h"

namespace mu::propertiespanel {
namespace BarlineTypes {
Q_NAMESPACE;
QML_ELEMENT;

enum class LineType {
    TYPE_NORMAL = int(engraving::BarLineType::NORMAL),
    TYPE_SINGLE = int(engraving::BarLineType::SINGLE),
    TYPE_DOUBLE = int(engraving::BarLineType::DOUBLE),
    TYPE_START_REPEAT = int(engraving::BarLineType::START_REPEAT),
    TYPE_LEFT_REPEAT = int(engraving::BarLineType::LEFT_REPEAT),
    TYPE_END_REPEAT = int(engraving::BarLineType::END_REPEAT),
    TYPE_RIGHT_REPEAT = int(engraving::BarLineType::RIGHT_REPEAT),
    TYPE_DASHED = int(engraving::BarLineType::BROKEN),
    TYPE_BROKEN = int(engraving::BarLineType::BROKEN),
    TYPE_FINAL = int(engraving::BarLineType::END),
    TYPE_END = int(engraving::BarLineType::END),
    TYPE_END_START_REPEAT = int(engraving::BarLineType::END_START_REPEAT),
    TYPE_LEFT_RIGHT_REPEAT = int(engraving::BarLineType::LEFT_RIGHT_REPEAT),
    TYPE_DOTTED = int(engraving::BarLineType::DOTTED),
    TYPE_REVERSE_END = int(engraving::BarLineType::REVERSE_END),
    TYPE_REVERS_FINAL = int(engraving::BarLineType::REVERSE_FINALE),
    TYPE_HEAVY = int(engraving::BarLineType::HEAVY),
    TYPE_DOUBLE_HEAVY = int(engraving::BarLineType::DOUBLE_HEAVY),
};
Q_ENUM_NS(LineType)

enum class SpanPreset {
    PRESET_DEFAULT,
    PRESET_TICK_1,
    PRESET_TICK_2,
    PRESET_SHORT_1,
    PRESET_SHORT_2,
};
Q_ENUM_NS(SpanPreset)

enum class AutoCustomHide {
    COUNT_AUTO = int(engraving::AutoCustomHide::AUTO),
    COUNT_CUSTOM = int(engraving::AutoCustomHide::CUSTOM),
    COUNT_HIDE = int(engraving::AutoCustomHide::HIDE)
};

Q_ENUM_NS(AutoCustomHide)
}
}
