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
#ifndef MU_DRAW_SVGRENDERER_H
#define MU_DRAW_SVGRENDERER_H

#include <QByteArray>

#include "geometry.h"
#include "painter.h"

class QSvgRenderer;

namespace mu::draw {
class SvgRenderer
{
public:
    SvgRenderer(const QByteArray& data);
    ~SvgRenderer();

    SizeF defaultSize() const;

    void render(Painter* painter, const RectF& rect);

private:
    QSvgRenderer* m_qSvgRenderer = nullptr;
};
}

#endif // MU_DRAW_SVGRENDERER_H
