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

#include "engraving/dom/measurenumber.h"
#include "engraving/dom/mmrestrange.h"

namespace mu::engraving::rendering::score {
class MeasureNumberLayout
{
public:
    static void layoutMeasureNumber(const MeasureNumber* item, MeasureNumber::LayoutData* ldata, const LayoutContext& ctx);
    static void layoutMMRestRange(const MMRestRange* item, MMRestRange::LayoutData* ldata, const LayoutContext& ctx);

private:
    static void layoutMeasureNumberBase(const MeasureNumberBase* item, MeasureNumberBase::LayoutData* ldata);

    static const Segment* refBarlineSegment(const MeasureNumber* item, bool alignToBarline, AlignH hPlacement);
    static void checkBarlineCollisions(const MeasureNumber* item, const Segment* barlineSeg, AlignH hPlacement,
                                       MeasureNumber::LayoutData* ldata);
};
}
