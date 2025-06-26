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
#ifndef MU_ENGRAVING_TEXTSTYLE_H
#define MU_ENGRAVING_TEXTSTYLE_H

#include "styledef.h"
#include "../types/types.h"

namespace mu::engraving {
enum class TextStylePropertyType : char {
    Undefined = 0,
    FontFace,
    FontSize,
    LineSpacing,
    SizeSpatiumDependent,
    FontStyle,
    Color,
    TextAlign,
    Offset,
    FrameType,
    FramePadding,
    FrameWidth,
    FrameRound,
    FrameBorderColor,
    FrameFillColor,
    MusicalSymbolsScale,
    Position
};

struct TextStyleProperty {
    TextStylePropertyType type;
    Sid sid;
    Pid pid;
};

constexpr size_t TEXT_STYLE_SIZE = 16;

typedef std::array<TextStyleProperty, TEXT_STYLE_SIZE> TextStyle;

const TextStyle* textStyle(TextStyleType);
const std::vector<TextStyleType>& allTextStyles();
const std::vector<TextStyleType>& editableTextStyles();
const std::vector<TextStyleType>& primaryTextStyles();
}

#endif // MU_ENGRAVING_TEXTSTYLE_H
