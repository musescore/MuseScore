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

#include "../dom/marker.h"
#include "rendering/score/layoutcontext.h"

namespace mu::engraving::rendering::score {
class LayoutConfiguration;

class MarkerLayout
{
public:
    static void layoutMarker(const Marker* item, Marker::LayoutData* ldata, LayoutContext& ctx);

private:
    static void doLayoutMarker(const Marker* item, Marker::LayoutData* ldata, LayoutContext& ctx);
    static double computeCustomTextOffset(const Marker* item, Marker::LayoutData* ldata, LayoutContext& ctx);
};
} // namespace mu::engraving::rendering::score
