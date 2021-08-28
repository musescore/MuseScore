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
    //onNotationChanged();
    //globalContext()->currentNotationChanged().onNotify(this, [this]() {
    //    onNotationChanged();
    //});
}

void PianorollScrollbar::updateSlider()
{
//    notation::INotationPtr notation = globalContext()->currentNotation();
//    if (!notation) {
//        return;
//    }

//    Ms::Score* score = notation->elements()->msScore();
//    Ms::Measure* lm = score->lastMeasure();
//    Ms::Fraction spanInNotesFrac = lm->tick() + lm->ticks();
//    double spanInNotes = spanInNotesFrac.numerator() / (double)spanInNotesFrac.denominator();
////    setImplicitSize((int)(beat * m_wholeNoteWidth), m_noteHeight * NUM_PITCHES);
//    int viewportWidth = width();
//    m_wholeNoteWidth;

//    m_focusWholeNotes;

//    double focusPix = viewportWidth * (m_focusWholeNotes / spanInNotes);

//    double scoreSpanPix = m_wholeNoteWidth * spanInNotes;
//    double scrollheadWidth = qMin(m_noteWindowWidth / scoreSpanPix, 1);

//    double scrollheadWidthPix = scrollheadWidth * viewportWidth;

    double frac = m_viewportSpan / m_displayObjectSpan;
    if (frac > 0)
        m_center = .5;

}

void PianorollScrollbar::setDirection(Direction value)
{
    if (value == m_direction)
        return;
    m_direction = value;

    emit directionChanged();
}

void PianorollScrollbar::setCenter(double value)
{
    if (value == m_center)
        return;
    m_center = value;

    emit centerChanged();

    updateCenter();
    update();
}

void PianorollScrollbar::updateCenter()
{
    //Clip center to bounds of slider

    if (m_viewportSpan >= m_displayObjectSpan)
    {
        setCenter(.5);
        return;
    }

    double frac = m_viewportSpan / m_displayObjectSpan;
    if (m_center < frac / 2)
        setCenter(frac / 2);
    else if (m_center > 1 - (frac / 2))
        setCenter(1 - (frac / 2));
}

void PianorollScrollbar::setViewportSpan(double value)
{
    if (value == m_viewportSpan)
        return;
    m_viewportSpan = value;

    emit viewportSpanChanged();
    updateCenter();
    update();
}

void PianorollScrollbar::setDisplayObjectSpan(double value)
{
    if (value == m_displayObjectSpan)
        return;
    m_displayObjectSpan = value;

    emit displayObjectSpanChanged();
    updateCenter();
    update();
}

void PianorollScrollbar::mousePressEvent(QMouseEvent *event)
{
    m_dragging = true;
    m_mouseDownPos = event->pos();
    m_centerDragStart = m_center;
}

void PianorollScrollbar::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;

}

void PianorollScrollbar::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging)
    {
        int delta = event->pos().x() - m_mouseDownPos.x();
        //delta / width()
        setCenter(m_centerDragStart + delta / (double)width());
    }

}

void PianorollScrollbar::paint(QPainter* p)
{
    p->fillRect(0, 0, width(), height(), m_colorBackground);

    double span = width() * m_viewportSpan / m_displayObjectSpan;
    p->fillRect(m_center * width() - span / 2, 0, span, height(), m_colorSlider);


}
