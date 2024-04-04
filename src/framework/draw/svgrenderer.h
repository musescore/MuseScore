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
#ifndef MUSE_DRAW_SVGRENDERER_H
#define MUSE_DRAW_SVGRENDERER_H

#include "global/types/bytearray.h"

#include "types/geometry.h"
#include "painter.h"

class QSvgRenderer;

namespace muse::draw {
class SvgRenderer
{
public:
    SvgRenderer(const ByteArray& data);
    ~SvgRenderer();

    SizeF defaultSize() const;

    void render(Painter* painter, const RectF& rect);

private:
    QSvgRenderer* m_qSvgRenderer = nullptr;
};
}

#endif // MUSE_DRAW_SVGRENDERER_H
