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

#include "../dom/dynamic.h"

namespace mu::engraving::rendering::score {
class LayoutConfiguration;

class DynamicsLayout
{
public:
    static void layoutDynamic(Dynamic* item, Dynamic::LayoutData* ldata, const LayoutConfiguration& conf);

private:
    static void doLayoutDynamic(Dynamic* item, Dynamic::LayoutData* ldata, const LayoutConfiguration& conf);
    static double computeCustomTextOffset(Dynamic* item, Dynamic::LayoutData* ldata, const LayoutConfiguration& conf);
    static void layoutDynamicToEndOfPrevious(const Dynamic* item, TextBase::LayoutData* ldata);
    static void manageBarlineCollisions(const Dynamic* item, TextBase::LayoutData* ldata);
};
} // namespace mu::engraving::rendering::score
