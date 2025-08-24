/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "indicatoricon.h"

#include "mscore.h"
#include "page.h"
#include "system.h"

using namespace mu::engraving;
using namespace muse::draw;

IndicatorIcon::IndicatorIcon(const ElementType& type, System* parent, ElementFlags flags)
    : EngravingItem(type, parent, flags)
{
}

Font IndicatorIcon::font() const
{
    Font font(configuration()->iconsFontFamily(), Font::Type::Icon);
    static constexpr double STANDARD_POINT_SIZE = 12.0;
    const double scaling = spatium() / SPATIUM20;
    font.setPointSizeF(STANDARD_POINT_SIZE * scaling);
    return font;
}
