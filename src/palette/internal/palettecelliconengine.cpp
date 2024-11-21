/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "palettecelliconengine.h"

#include <QPainter>

#include "draw/types/geometry.h"
#include "draw/painter.h"
#include "draw/types/pen.h"
#include "engraving/dom/actionicon.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/masterscore.h"
#include "engraving/style/defaultstyle.h"

#include "notation/utilities/engravingitempreviewpainter.h"

#include "log.h"

using namespace mu::palette;
using namespace muse::draw;
using namespace mu::engraving;

PaletteCellIconEngine::PaletteCellIconEngine(PaletteCellConstPtr cell, qreal extraMag)
    : QIconEngine(), m_cell(cell), m_extraMag(extraMag)
{
}

QIconEngine* PaletteCellIconEngine::clone() const
{
    return new PaletteCellIconEngine(m_cell, m_extraMag);
}

void PaletteCellIconEngine::paint(QPainter* qp, const QRect& rect, QIcon::Mode mode, QIcon::State state)
{
    qreal dpi = qp->device()->logicalDpiX();
    Painter p(qp, "palettecell");
    p.save();
    p.setAntialiasing(true);
    paintCell(p, RectF::fromQRectF(rect), mode == QIcon::Selected, state == QIcon::On, dpi);
    p.restore();
}

void PaletteCellIconEngine::paintCell(Painter& painter, const RectF& rect, bool selected, bool current, qreal dpi) const
{
    paintBackground(painter, rect, selected, current);

    if (!m_cell) {
        return;
    }

    EngravingItem* element = m_cell->element.get();
    if (!element) {
        return;
    }

    notation::EngravingItemPreviewPainter::PaintParams params;
    params.painter = &painter;

    params.color = configuration()->elementsColor();

    params.mag = m_extraMag * m_cell->mag;
    params.xoffset = m_cell->xoffset;
    params.yoffset = m_cell->yoffset;

    params.rect = rect;
    params.dpi = dpi;
    params.spatium = configuration()->paletteSpatium() * params.mag;

    params.drawStaff = m_cell->drawStaff;

    notation::EngravingItemPreviewPainter::paintPreview(element, params);
}

void PaletteCellIconEngine::paintBackground(Painter& painter, const RectF& rect, bool selected, bool current) const
{
    if (current || selected) {
        QColor c(configuration()->accentColor());
        c.setAlpha(selected ? 100 : 60);
        painter.fillRect(rect, c);
    }
}
