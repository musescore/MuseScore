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
#ifndef MU_ENGRAVING_TEXTSTYLE_H
#define MU_ENGRAVING_TEXTSTYLE_H

#include "styledef.h"
#include "types/types.h"

namespace Ms {
#define TEXT_STYLE_SIZE 14

typedef std::array<StyledProperty, TEXT_STYLE_SIZE> TextStyle;

const TextStyle* textStyle(TextStyleType);
const std::vector<TextStyleType>& allTextStyles();
const std::vector<TextStyleType>& primaryTextStyles();
}

#endif // MU_ENGRAVING_TEXTSTYLE_H
