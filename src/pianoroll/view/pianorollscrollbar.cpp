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

#include "pianorollscrollbar.h"

#include <qpainter.h>

using namespace mu::pianoroll;

PianorollScrollbar::PianorollScrollbar(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
}

void PianorollScrollbar::load()
{
}

void PianorollScrollbar::updateSlider()
{
    double frac = m_viewportSpan / m_displayObjectSpan;
    if (frac > 0) {
        m_center = .5;
    }
}

void PianorollScrollbar::setDirection(Direction value)
{
    if (value == m_direction) {
        return;
    }
    m_direction = value;

    emit directionChanged();
    update();
}

void PianorollScrollbar::setCenter(double value)
{
    if (value == m_center) {
        return;
    }
    m_center = value;

    emit centerChanged();

    updateCenter();
    update();
}

void PianorollScrollbar::updateCenter()
{
    //Clip center to bounds of slider

    if (m_viewportSpan >= m_displayObjectSpan) {
        setCenter(.5);
        return;
    }

    double frac = m_viewportSpan / m_displayObjectSpan;
    if (m_center < frac / 2) {
        setCenter(frac / 2);
    } else if (m_center > 1 - (frac / 2)) {
        setCenter(1 - (frac / 2));
    }
}

void PianorollScrollbar::setViewportSpan(double value)
{
    if (value == m_viewportSpan) {
        return;
    }
    m_viewportSpan = value;

    emit viewportSpanChanged();
    updateCenter();
    update();
}

void PianorollScrollbar::setDisplayObjectSpan(double value)
{
    if (value == m_displayObjectSpan) {
        return;
    }
    m_displayObjectSpan = value;

    emit displayObjectSpanChanged();
    updateCenter();
    update();
}

void PianorollScrollbar::wheelEvent(QWheelEvent* event)
{
    QPoint scroll = event->angleDelta();
    double amount = -(double)scroll.y() / 400;

    setCenter(m_center + amount * m_viewportSpan / m_displayObjectSpan);
}

void PianorollScrollbar::mousePressEvent(QMouseEvent* event)
{
    if (m_direction == Direction::HORIZONTAL) {
        double span = width() * m_viewportSpan / m_displayObjectSpan;
        double bound0 = m_center * width() - span / 2;
        double bound1 = bound0 + span;

        double p0 = event->pos().x();
        if (p0 < bound0) {
            setCenter(m_center - m_viewportSpan / m_displayObjectSpan);
            return;
        }
        else if (p0 > bound1) {
            setCenter(m_center + m_viewportSpan / m_displayObjectSpan);
            return;
        }
    }
    else {
        double span = height() * m_viewportSpan / m_displayObjectSpan;
        double bound0 = m_center * height() - span / 2;
        double bound1 = bound0 + span;

        double p0 = event->pos().y();
        if (p0 < bound0) {
            setCenter(m_center - m_viewportSpan / m_displayObjectSpan);
            return;
        }
        else if (p0 > bound1) {
            setCenter(m_center + m_viewportSpan / m_displayObjectSpan);
            return;
        }
    }

    m_dragging = true;
    m_mouseDownPos = event->pos();
    m_centerDragStart = m_center;
}

void PianorollScrollbar::mouseReleaseEvent(QMouseEvent* event)
{
    m_dragging = false;
}

void PianorollScrollbar::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging) {
        int delta = m_direction == Direction::HORIZONTAL
                    ? event->pos().x() - m_mouseDownPos.x()
                    : event->pos().y() - m_mouseDownPos.y();
        double span = m_direction == Direction::HORIZONTAL ? width() : height();
        setCenter(m_centerDragStart + delta / span);
    }
}

void PianorollScrollbar::paint(QPainter* p)
{
    p->fillRect(0, 0, width(), height(), m_colorBackground);

    if (m_direction == Direction::HORIZONTAL) {
        double span = width() * m_viewportSpan / m_displayObjectSpan;
        QRect bounds(m_center * width() - span / 2, 0, span, height());
        p->fillRect(bounds, m_colorSlider);
        p->setPen(m_colorSlider.darker(100));
        p->drawRect(bounds);
    } else {
        double span = height() * m_viewportSpan / m_displayObjectSpan;
        QRect bounds(0, m_center * height() - span / 2, width(), span);
        p->fillRect(bounds, m_colorSlider);
        p->setPen(m_colorSlider.darker(100));
        p->drawRect(bounds);
    }
}
