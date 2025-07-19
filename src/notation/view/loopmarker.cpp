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

void LoopMarker::move(muse::midi::tick_t tick)
{
    m_rect = resolveMarkerRectByTick(tick);
}

RectF LoopMarker::resolveMarkerRectByTick(muse::midi::tick_t _tick) const
{
    if (!m_notation) {
        return RectF();
    }

    const mu::engraving::Score* score = m_notation->elements()->msScore();

    Fraction tick = Fraction::fromTicks(_tick);

    // set mark height for whole system
    if (m_type == LoopBoundaryType::LoopOut && tick > Fraction(0, 1)) {
        tick -= Fraction::fromTicks(1);
    }

    Measure* measure = score->tick2measureMM(tick);
    if (measure == nullptr) {
        return RectF();
    }

    qreal x = 0.0;
    const Fraction offset = { 0, 1 };

    mu::engraving::Segment* s = nullptr;
    for (s = measure->first(mu::engraving::SegmentType::ChordRest); s;) {
        Fraction t1 = s->tick();
        int x1 = s->canvasPos().x();
        qreal x2 = 0.0;
        Fraction t2;
        mu::engraving::Segment* ns = s->next(mu::engraving::SegmentType::ChordRest);

        if (ns) {
            t2 = ns->tick();
            x2 = ns->canvasPos().x();
        } else {
            t2 = measure->endTick();
            x2 = measure->canvasPos().x() + measure->width();
        }

        t1 += offset;
        t2 += offset;

        if (tick >= t1 && tick < t2) {
            Fraction dt = t2 - t1;
            qreal dx = x2 - x1;
            x = x1 + dx * (tick - t1).ticks() / dt.ticks();
            break;
        }

        s = ns;
    }

    if (s == nullptr) {
        return RectF();
    }

    const mu::engraving::System* system = measure->system();
    if (system == nullptr || system->page() == nullptr || system->staves().empty()) {
        return RectF();
    }

    double y = system->staffYpage(0) + system->page()->pos().y();
    double _spatium = score->style().spatium();

    qreal mag = _spatium / mu::engraving::SPATIUM20;
    double width = (_spatium * 2.0 + score->engravingFont()->width(mu::engraving::SymId::noteheadBlack, mag)) / 3;
    double height = 6 * _spatium;

    // set cursor height for whole system
    double y2 = 0.0;

    for (size_t i = 0; i < score->nstaves(); ++i) {
        mu::engraving::SysStaff* ss = system->staff(i);
        if (!ss->show() || !score->staff(i)->show()) {
            continue;
        }
        y2 = ss->y() + ss->bbox().height();
    }

    height += y2;
    y -= 3 * _spatium;

    if (m_type == LoopBoundaryType::LoopIn) {
        x = x - _spatium + width / 1.5;
    } else {
        x = x - _spatium * .5;
    }

    return RectF(x, y, width, height);
}

void LoopMarker::paint(muse::draw::Painter* painter)
{
    using namespace muse::draw;

    if (!m_visible || !m_notation) {
        return;
    }

    PolygonF triangle(3);

    qreal x = m_rect.left();
    qreal y = m_rect.top();
    qreal h = m_notation->style()->styleValue(StyleId::spatium).toDouble() * 2;

    QColor color = configuration()->loopMarkerColor();

    switch (m_type) {
    case LoopBoundaryType::LoopIn: { // draw a right-pointing triangle
        qreal tx = x - 1.0;
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

    painter->setPen(Pen(color, 2.0, PenStyle::SolidLine, PenCapStyle::FlatCap, PenJoinStyle::MiterJoin));
    painter->drawLine(x, y, x, m_rect.bottom());
    painter->setBrush(color);
    painter->drawConvexPolygon(triangle);
}
