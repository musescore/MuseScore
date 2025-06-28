/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include "layoutcontext.h"

#include "dom/tapping.h"

namespace mu::engraving::rendering::score {
class TappingLayout
{
public:
    static void layoutTapping(Tapping* item, Tapping::LayoutData* ldata, LayoutContext& ctx);

private:
    static void layoutLeftHandTapping(Tapping* item, Tapping::LayoutData* ldata, const MStyle& style, bool tabStaff, LayoutContext& ctx);
    static void updateHalfSlurs(Tapping* item, const MStyle& style, bool tabStaff, LayoutContext& ctx);
    static void layoutHalfSlur(Tapping* item, TappingHalfSlur* slur, LayoutContext& ctx);

    static void layoutRightHandTapping(Tapping* item, Tapping::LayoutData* ldata, const MStyle& style, bool tabStaff, LayoutContext& ctx);
};
}
