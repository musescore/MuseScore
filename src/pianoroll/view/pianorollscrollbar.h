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

#ifndef MU_PIANOROLL_PIANOROLLSCROLLBAR_H
#define MU_PIANOROLL_PIANOROLLSCROLLBAR_H

#include <QQuickPaintedItem>
#include <QIcon>
#include <QColor>
#include <QWheelEvent>

#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "pianoroll/ipianorollcontroller.h"

namespace mu::pianoroll {
class PianorollScrollbar : public QQuickPaintedItem, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(Direction diretion READ direction WRITE setDirection NOTIFY directionChanged)
    Q_PROPERTY(double center READ center WRITE setCenter NOTIFY centerChanged)
    Q_PROPERTY(double viewportSpan READ viewportSpan WRITE setViewportSpan NOTIFY viewportSpanChanged)
    Q_PROPERTY(double displayObjectSpan READ displayObjectSpan WRITE setDisplayObjectSpan NOTIFY displayObjectSpanChanged)

public:
    enum class Direction : char {
        HORIZONTAL, VERTICAL
    };
    Q_ENUM(Direction)

public:
    PianorollScrollbar(QQuickItem* parent = nullptr);

    Q_INVOKABLE void load();

    void wheelEvent(QWheelEvent* event);
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

    Direction direction() const { return m_direction; }
    void setDirection(Direction value);
    double center() const { return m_center; }
    void setCenter(double value);
    double viewportSpan() const { return m_viewportSpan; }
    void setViewportSpan(double value);
    double displayObjectSpan() const { return m_displayObjectSpan; }
    void setDisplayObjectSpan(double value);

    void updateCenter();
    void paint(QPainter*) override;

signals:
    void directionChanged();
    void centerChanged();
    void viewportSpanChanged();
    void displayObjectSpanChanged();

private:
    void updateSlider();

    QPoint m_mouseDownPos;
    bool m_dragging = false;
    double m_centerDragStart;

    double m_displayObjectSpan = 0;  //Logical size of object inside the viewport in pixels
    double m_viewportSpan = 0;  //Viewport size (number of pixels displayed onscreen)
    double m_center = 0;

    Direction m_direction = Direction::HORIZONTAL;

    QColor m_colorSlider = Qt::gray;
    QColor m_colorBackground = QColor(0xdddddd);
};
}

#endif // MU_PIANOROLL_PIANOROLLSCROLLBAR_H
