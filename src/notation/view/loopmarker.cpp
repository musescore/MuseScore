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

#include "loopmarker.h"
#include "draw/types/pen.h"
#include "engraving/iengravingfont.h"

using namespace muse;
using namespace mu;
using namespace mu::notation;

LoopMarker::LoopMarker(LoopBoundaryType type, const modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx), m_type(type)
{
}

void LoopMarker::setNotation(INotationPtr notation)
{
    m_notation = notation;
}

void LoopMarker::setVisible(bool visible)
{
    m_visible = visible;
}

void LoopMarker::updatePosition(engraving::Fraction tick)
{
    m_rect = resolveMarkerRectByTick(tick);
}

RectF LoopMarker::resolveMarkerRectByTick(engraving::Fraction tick) const
{
    if (!m_notation) {
        return RectF();
    }

    const mu::engraving::Score* score = m_notation->elements()->msScore();

    // set mark height for whole system
    if (m_type == LoopBoundaryType::LoopOut && tick > Fraction(0, 1)) {
        tick -= Fraction::fromTicks(1);
    }

    const Measure* measure = score->tick2measureMM(tick);
    if (!measure) {
        return RectF();
    }

    const mu::engraving::System* system = measure->system();
    if (!system || !system->page() || system->staves().empty()) {
        return RectF();
    }

    double x = 0.0;
    mu::engraving::Segment* s = nullptr;
    for (s = measure->first(mu::engraving::SegmentType::ChordRest); s;) {
        Fraction t1 = s->tick();
        int x1 = s->canvasPos().x();
        double x2 = 0.0;
        Fraction t2;

        mu::engraving::Segment* ns = s->next(mu::engraving::SegmentType::ChordRest);
        while (ns && !ns->visible()) {
            ns = ns->next(mu::engraving::SegmentType::ChordRest);
        }

        if (ns) {
            t2 = ns->tick();
            x2 = ns->canvasPos().x();
        } else {
            t2 = measure->endTick();
            // measure->width is not good enough because of courtesy keysig, timesig
            const mu::engraving::Segment* seg = measure->findSegment(mu::engraving::SegmentType::EndBarLine, measure->endTick());
            if (seg) {
                x2 = seg->canvasPos().x();
            } else {
                x2 = measure->canvasPos().x() + measure->width(); // safety, should not happen
            }
        }

        if (tick >= t1 && tick < t2) {
            Fraction dt = t2 - t1;
            double dx = x2 - x1;
            x = x1 + dx * (tick - t1).ticks() / dt.ticks();
            break;
        }
        s = ns;
    }

    if (!s) {
        return RectF();
    }

    const double _spatium = score->style().spatium();
    const double mag = _spatium / score->style().defaultSpatium();

    const double width = (_spatium * 2.0 + score->engravingFont()->width(mu::engraving::SymId::noteheadBlack, mag)) / 3;

    if (m_type == LoopBoundaryType::LoopIn) {
        x = x - _spatium + width / 1.5;
    } else {
        x = x - _spatium * .5;
    }

    const double y = system->staffCanvasYpage(0) - 3 * _spatium;

    // set cursor height for whole system
    double y2 = 0.0;

    for (size_t i = 0; i < score->nstaves(); ++i) {
        mu::engraving::SysStaff* ss = system->staff(i);
        if (!ss->show() || !score->staff(i)->show()) {
            continue;
        }
        y2 = ss->y() + ss->bbox().height();
    }

    const double height = y2 + 6 * _spatium;

    return RectF { x, y, width, height };
}

void LoopMarker::paint(muse::draw::Painter* painter)
{
    using namespace muse::draw;

    if (!m_visible || !m_notation) {
        return;
    }

    const mu::engraving::Score* score = m_notation->elements()->msScore();
    const double spatium = score->style().spatium();

    PolygonF triangle(3);

    const double x = m_rect.left();
    const double y = m_rect.top();
    const double h = m_notation->style()->styleValue(StyleId::spatium).toDouble() * 2;

    const QColor color = configuration()->loopMarkerColor();

    switch (m_type) {
    case LoopBoundaryType::LoopIn: { // draw a right-pointing triangle
        const double tx = x - 1.0;
        triangle[0] = PointF(tx, y);
        triangle[1] = PointF(tx, y + h);
        triangle[2] = PointF(tx + h, y + h / 2);
    }
    break;
    case LoopBoundaryType::LoopOut: { // draw a left-pointing triangle
        triangle[0] = PointF(x, y);
        triangle[1] = PointF(x, y + h);
        triangle[2] = PointF(x - h, y + h / 2);
    }
    break;
    case LoopBoundaryType::Unknown: return;
    }

    const double lineWidth = 0.15 * spatium;

    painter->setPen(Pen(color, lineWidth, PenStyle::SolidLine, PenCapStyle::FlatCap, PenJoinStyle::MiterJoin));
    painter->drawLine(x, y, x, m_rect.bottom());
    painter->setBrush(color);
    painter->drawConvexPolygon(triangle);
}
