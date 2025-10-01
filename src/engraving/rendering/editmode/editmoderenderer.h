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
#pragma once

#include "../ieditmoderenderer.h"

namespace muse::draw {
class Painter;
}

namespace mu::engraving {
class MStyle;
class IEngravingFont;
class Score;
class EngravingItem;
class EditData;

class BarLine;
class Dynamic;
class SlurTieSegment;
class TextBase;
}

namespace mu::engraving::rendering::editmode {
class EditModeRenderer : public IEditModeRenderer
{
public:
    EditModeRenderer() = default;

    void drawItem(const EngravingItem* item, muse::draw::Painter* p, const EditData& ed, double currentViewScaling,
                  const PaintOptions& opt) override;

private:
    static void drawEngravingItem(const EngravingItem* item, muse::draw::Painter* painter, const EditData& ed, double currentViewScaling,
                                  const PaintOptions& opt);

    static void drawBarline(const BarLine* item, muse::draw::Painter* painter, const EditData& ed, double currentViewScaling,
                            const PaintOptions& opt);
    static void drawDynamic(const Dynamic* item, muse::draw::Painter* painter, const EditData& ed, double currentViewScaling,
                            const PaintOptions& opt);
    static void drawSlurTieSegment(const SlurTieSegment* item, muse::draw::Painter* painter, const EditData& ed, double currentViewScaling,
                                   const PaintOptions& opt);
    static void drawTextBase(const TextBase* item, muse::draw::Painter* painter, const EditData& ed, double currentViewScaling,
                             const PaintOptions& opt);
};
}
