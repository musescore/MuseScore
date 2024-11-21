/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "modularity/ioc.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/rendering/isinglerenderer.h"

namespace mu::notation {
class EngravingItemPreviewPainter
{
    INJECT_STATIC(engraving::rendering::ISingleRenderer, engravingRender)

public:
    struct PaintParams
    {
        muse::draw::Painter* painter = nullptr;

        mu::engraving::Color color;

        double mag = 1.0;
        double xoffset = 0.0;
        double yoffset = 0.0;

        muse::RectF rect;
        double dpi = 1.0;
        double spatium = 1.0;

        bool useElementColors = false;
        bool colorsInversionEnabled = false;
        bool drawStaff = false;
    };

    static void paintPreview(mu::engraving::EngravingItem* element, PaintParams& params);
    static void paintItem(mu::engraving::EngravingItem* element, PaintParams& params);

private:
    static void paintPreviewForItem(mu::engraving::EngravingItem* element, PaintParams& params);
    static void paintPreviewForActionIcon(mu::engraving::EngravingItem* element, PaintParams& params);
    static double paintStaff(PaintParams& params);
};
}
