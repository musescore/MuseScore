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
#include "palettecelliconengine.h"

#include "engraving/draw/geometry.h"
#include "engraving/draw/painter.h"
#include "engraving/draw/pen.h"
#include "engraving/libmscore/actionicon.h"
#include "engraving/libmscore/element.h"
#include "engraving/libmscore/masterscore.h"
#include "engraving/style/defaultstyle.h"

#include "log.h"

using namespace mu::palette;
using namespace mu::draw;
using namespace Ms;

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
    Painter p(qp, "palettecell");
    p.save();
    p.setAntialiasing(true);
    paintCell(p, RectF::fromQRectF(rect), mode == QIcon::Selected, state == QIcon::On);
    p.restore();
}

void PaletteCellIconEngine::paintCell(Painter& painter, const RectF& rect, bool selected, bool current) const
{
    double guiScaling = uiConfiguration()->guiScaling();
    painter.scale(guiScaling, guiScaling);

    paintBackground(painter, rect, selected, current);

    if (!m_cell) {
        return;
    }

    Element* element = m_cell->element.get();
    if (!element) {
        return;
    }

    painter.setPen(configuration()->elementsColor());

    if (element->isActionIcon()) {
        paintActionIcon(painter, rect, element);
        return; // never draw staff for icon elements
    }

    const bool drawStaff = m_cell->drawStaff;
    const qreal spatium = configuration()->paletteSpatium() * m_extraMag * m_cell->mag;

    PointF origin = rect.center(); // draw element at center of cell by default
    if (drawStaff) {
        const qreal topLinePos = paintStaff(painter, rect, spatium); // draw dummy staff lines onto rect.
        origin.setY(topLinePos); // vertical position relative to staff instead of cell center.
    }

    painter.translate(origin);
    painter.translate(m_cell->xoffset * spatium, m_cell->yoffset * spatium); // additional offset for element onlym

    paintScoreElement(painter, element, spatium, drawStaff);
}

void PaletteCellIconEngine::paintBackground(Painter& painter, const RectF& rect, bool selected, bool current) const
{
    if (current || selected) {
        QColor c(configuration()->accentColor());
        c.setAlpha(selected ? 100 : 60);
        painter.fillRect(rect, c);
    }
}

/// Paint an icon element so that it fills a QRect, preserving aspect ratio, and
/// leaving a small margin around the edges.
void PaletteCellIconEngine::paintActionIcon(Painter& painter, const RectF& rect, Element* element) const
{
    IF_ASSERT_FAILED(element && element->isActionIcon()) {
        return;
    }

    painter.save();

    constexpr qreal margin = 4.0;
    qreal extent = qMin(rect.height(), rect.width()) - margin;

    ActionIcon* action = toActionIcon(element);
    action->setExtent(extent);

    extent /= 2.0;
    PointF iconCenter(extent, extent);

    painter.translate(rect.center() - iconCenter);
    action->draw(&painter);
    painter.restore();
}

/// Paint a 5 line staff centered within a QRect and return the distance from the
/// top of the QRect to the uppermost staff line.
qreal PaletteCellIconEngine::paintStaff(Painter& painter, const RectF& rect, qreal spatium) const
{
    painter.save();

    Pen pen(configuration()->elementsColor());
    pen.setWidthF(engraving::DefaultStyle::defaultStyle().value(Sid::staffLineWidth).toDouble() * spatium);
    painter.setPen(pen);

    constexpr int numStaffLines = 5;
    const qreal staffHeight = spatium * (numStaffLines - 1);
    const qreal topLineDist = rect.center().y() - (staffHeight / 2.0);

    // lines bounded horizontally by edge of target (with small margin)
    constexpr qreal margin = 3.0;
    const qreal x1 = rect.left() + margin;
    const qreal x2 = rect.right() - margin;

    // draw staff lines with middle line centered vertically on target
    qreal y = topLineDist;
    for (int i = 0; i < numStaffLines; ++i) {
        painter.drawLine(LineF(x1, y, x2, y));
        y += spatium;
    }

    painter.restore();
    return topLineDist;
}

/// Paint a non-icon element centered at the origin of the painter's coordinate
/// system. If alignToStaff is true then the element is only centered horizontally;
/// i.e. vertical alignment is unchanged from the default so that item will appear
/// at the correct height on the staff.
void PaletteCellIconEngine::paintScoreElement(Painter& painter, Element* element, qreal spatium, bool alignToStaff) const
{
    IF_ASSERT_FAILED(element && !element->isActionIcon()) {
        return;
    }

    painter.save();

    const qreal sizeRatio = spatium / gscore->spatium();
    painter.scale(sizeRatio, sizeRatio); // scale coordinates so element is drawn at correct size

    element->layout(); // calculate bbox
    PointF origin = element->bbox().center();

    if (alignToStaff) {
        // y = 0 is position of the element's parent.
        // If the parent is the staff (or a segment on the staff) then
        // y = 0 corresponds to the position of the top staff line.
        origin.setY(0.0);
    }

    painter.translate(-1.0 * origin); // shift coordinates so element is drawn at correct position

    element->scanElements(&painter, paintPaletteElement);
    painter.restore();
}

void PaletteCellIconEngine::paintPaletteElement(void* data, Element* element)
{
    Painter* painter = static_cast<Painter*>(data);
    painter->save();
    painter->translate(element->pos()); // necessary for drawing child elements

    auto colorBackup = Color::fromQColor(element->getProperty(Pid::COLOR).value<QColor>());
    auto frameColorBackup = Color::fromQColor(element->getProperty(Pid::FRAME_FG_COLOR).value<QColor>());

    auto color = Color::fromQColor(configuration()->elementsColor());
    element->setProperty(Pid::COLOR, QVariant::fromValue(color));
    element->setProperty(Pid::FRAME_FG_COLOR, QVariant::fromValue(color));

    element->draw(painter);

    element->setProperty(Pid::COLOR, QVariant::fromValue(colorBackup));
    element->setProperty(Pid::FRAME_FG_COLOR, QVariant::fromValue(frameColorBackup));

    painter->restore();
}
