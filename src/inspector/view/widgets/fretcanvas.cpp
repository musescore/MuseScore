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

#include "fretcanvas.h"

#include <cmath>

#include "libmscore/fret.h"
#include "libmscore/measure.h"
#include "libmscore/system.h"
#include "libmscore/score.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/segment.h"
#include "libmscore/undo.h"

using namespace mu::inspector;

//---------------------------------------------------------
//   FretCanvas
//---------------------------------------------------------

FretCanvas::FretCanvas(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);

    m_cstring = -2;
    m_cfret   = -2;
}

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void FretCanvas::draw(QPainter* painter)
{
    double mag        = 1.5;
    double _spatium   = 20.0 * mag;
    double lw1        = _spatium * 0.08;
    int fretOffset    = m_diagram->fretOffset();
    double lw2        = (fretOffset || !m_diagram->showNut()) ? lw1 : _spatium * 0.2;
    double stringDist = _spatium * .7;
    double fretDist   = _spatium * .8;
    int _strings      = m_diagram->strings();
    int _frets        = m_diagram->frets();
    double dotd       = stringDist * .6 + lw1;

    double w  = (_strings - 1) * stringDist;
    double xo = (width() - w) * .5;
    double h  = (_frets * fretDist) + fretDist * .5;
    double yo = (height() - h) * .5;

    QFont font("FreeSans");
    int size = lrint(18.0 * mag);
    font.setPixelSize(size);

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->translate(xo, yo);

    QPen pen(painter->pen());
    pen.setWidthF(lw2);
    pen.setCapStyle(Qt::FlatCap);
    pen.setColor(color());
    painter->setPen(pen);
    painter->setBrush(pen.color());
    double x2 = (_strings - 1) * stringDist;
    painter->drawLine(QLineF(-lw1 * .5, 0.0, x2 + lw1 * .5, 0.0));

    pen.setWidthF(lw1);
    painter->setPen(pen);
    double y2 = (_frets + 1) * fretDist - fretDist * .5;

    QPen symPen(pen);
    symPen.setCapStyle(Qt::RoundCap);
    symPen.setWidthF(lw1 * 1.2);

    // Draw strings and frets
    for (int i = 0; i < _strings; ++i) {
        double x = stringDist * i;
        painter->drawLine(QLineF(x, fretOffset ? -_spatium * .2 : 0.0, x, y2));
    }
    for (int i = 1; i <= _frets; ++i) {
        double y = fretDist * i;
        painter->drawLine(QLineF(0.0, y, x2, y));
    }

    // Draw dots and markers
    for (int i = 0; i < _strings; ++i) {
        for (auto const& dt : m_diagram->dot(i)) {
            if (dt.exists()) {
                painter->setPen(symPen);
                int fret = dt.fret;
                double x = stringDist * i - dotd * .5;
                double y = fretDist * (fret - 1) + fretDist * .5 - dotd * .5;

                paintDotSymbol(painter, symPen, x, y, dotd, dt.dtype);
            }
        }
        painter->setPen(pen);

        Ms::FretItem::Marker mark = m_diagram->marker(i);
        if (mark.exists()) {
            painter->setFont(font);
            double x = stringDist * i;
            double y = -fretDist * .1;
            painter->drawText(QRectF(x, y, 0.0, 0.0),
                              Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip,
                              Ms::FretItem::markerToChar(mark.mtype));
        }
    }

    // Draw barres
    painter->setPen(pen);
    for (auto const& i : m_diagram->barres()) {
        int fret        = i.first;
        int startString = i.second.startString;
        int endString   = i.second.endString;

        qreal x1   = stringDist * startString;
        qreal newX2 = endString == -1 ? x2 : stringDist * endString;

        qreal y    = fretDist * (fret - 1) + fretDist * .5;
        pen.setWidthF(dotd * m_diagram->score()->styleD(Ms::Sid::barreLineWidth));          // don't use style barreLineWidth - why not?
        pen.setCapStyle(Qt::RoundCap);
        painter->setPen(pen);
        painter->drawLine(QLineF(x1, y, newX2, y));
    }

    // Draw 'hover' dot
    if ((m_cfret > 0) && (m_cfret <= _frets) && (m_cstring >= 0) && (m_cstring < _strings)) {
        Ms::FretItem::Dot cd = m_diagram->dot(m_cstring, m_cfret)[0];
        std::vector<Ms::FretItem::Dot> otherDots = m_diagram->dot(m_cstring);
        Ms::FretDotType dtype;
        symPen.setColor(Qt::lightGray);

        if (cd.exists()) {
            dtype = cd.dtype;
            symPen.setColor(Qt::red);
        } else {
            dtype = m_automaticDotType ? Ms::FretDotType::NORMAL : m_currentDtype;
        }
        painter->setPen(symPen);

        double x = stringDist * m_cstring - dotd * .5;
        double y = fretDist * (m_cfret - 1) + fretDist * .5 - dotd * .5;
        painter->setBrush(Qt::lightGray);
        paintDotSymbol(painter, symPen, x, y, dotd, dtype);
    }

    if (fretOffset > 0) {
        qreal fretNumMag = 2.0;     // TODO: get the value from Sid::fretNumMag
        QFont scaledFont(font);
        scaledFont.setPixelSize(font.pixelSize() * fretNumMag);
        painter->setFont(scaledFont);
        painter->setPen(pen);
        // Todo: make dependent from Sid::fretNumPos
        painter->drawText(QRectF(-stringDist * .4, 0.0, 0.0, fretDist),
                          Qt::AlignVCenter | Qt::AlignRight | Qt::TextDontClip,
                          QString("%1").arg(fretOffset + 1));
        painter->setFont(font);
    }
}

//---------------------------------------------------------
//   paintDotSymbol
//---------------------------------------------------------

void FretCanvas::paintDotSymbol(QPainter* p, QPen& pen, qreal x, qreal y, qreal dotd, Ms::FretDotType dtype)
{
    switch (dtype) {
    case Ms::FretDotType::CROSS:
        p->drawLine(QLineF(x, y, x + dotd, y + dotd));
        p->drawLine(QLineF(x + dotd, y, x, y + dotd));
        break;
    case Ms::FretDotType::SQUARE:
        p->setBrush(Qt::NoBrush);
        p->drawRect(QRectF(x, y, dotd, dotd));
        break;
    case Ms::FretDotType::TRIANGLE:
        p->drawLine(QLineF(x, y + dotd, x + .5 * dotd, y));
        p->drawLine(QLineF(x + .5 * dotd, y, x + dotd, y + dotd));
        p->drawLine(QLineF(x + dotd, y + dotd, x, y + dotd));
        break;
    case Ms::FretDotType::NORMAL:
    default:
        p->setBrush(pen.color());
        p->setPen(Qt::NoPen);
        p->drawEllipse(QRectF(x, y, dotd, dotd));
        break;
    }
}

//---------------------------------------------------------
//   getPosition
//---------------------------------------------------------

void FretCanvas::getPosition(const QPointF& p, int* string, int* fret)
{
    double mag = 1.5;
    double _spatium   = 20.0 * mag;
    int _strings      = m_diagram->strings();
    int _frets        = m_diagram->frets();
    double stringDist = _spatium * .7;
    double fretDist   = _spatium * .8;

    double w  = (_strings - 1) * stringDist;
    double xo = (width() - w) * .5;
    double h  = (_frets * fretDist) + fretDist * .5;
    double yo = (height() - h) * .5;
    *fret  = (p.y() - yo + fretDist) / fretDist;
    *string = (p.x() - xo + stringDist * .5) / stringDist;
}

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void FretCanvas::mousePressEvent(QMouseEvent* ev)
{
    int string;
    int fret;
    getPosition(ev->pos(), &string, &fret);

    int _strings = m_diagram->strings();
    int _frets   = m_diagram->frets();
    if (fret < 0 || fret > _frets || string < 0 || string >= _strings) {
        return;
    }

    globalContext()->currentNotation()->undoStack()->prepareChanges();

    // Click above the fret diagram, so change the open/closed string marker
    if (fret == 0) {
        switch (m_diagram->marker(string).mtype) {
        case Ms::FretMarkerType::CIRCLE:
            m_diagram->undoSetFretMarker(string, Ms::FretMarkerType::CROSS);
            break;
        case Ms::FretMarkerType::CROSS:
            m_diagram->undoSetFretMarker(string, Ms::FretMarkerType::NONE);
            break;
        case Ms::FretMarkerType::NONE:
        default:
            m_diagram->undoSetFretDot(string, 0);
            m_diagram->undoSetFretMarker(string, Ms::FretMarkerType::CIRCLE);
            break;
        }
    }
    // Otherwise, the click is on the fretboard itself
    else {
        Ms::FretItem::Dot thisDot = m_diagram->dot(string, fret)[0];
        bool haveShift = (ev->modifiers() & Qt::ShiftModifier) || m_barreMode;
        bool haveCtrl  = (ev->modifiers() & Qt::ControlModifier) || m_multidotMode;

        // Click on an existing dot
        if (thisDot.exists() && !haveShift) {
            m_diagram->undoSetFretDot(string, haveCtrl ? fret : 0, haveCtrl);
        } else {
            // Shift adds a barre
            if (haveShift) {
                m_diagram->undoSetFretBarre(string, fret, haveCtrl);
            } else {
                Ms::FretDotType dtype = Ms::FretDotType::NORMAL;
                if (m_automaticDotType && haveCtrl && m_diagram->dot(string)[0].exists()) {
                    dtype = Ms::FretDotType::TRIANGLE;

                    std::vector<Ms::FretDotType> dtypes {
                        Ms::FretDotType::NORMAL,
                        Ms::FretDotType::CROSS,
                        Ms::FretDotType::SQUARE,
                        Ms::FretDotType::TRIANGLE
                    };

                    // Find the lowest dot type that doesn't already exist on the string
                    for (int i = 0; i < int(dtypes.size()); i++) {
                        Ms::FretDotType type = dtypes[i];

                        bool hasThisType = false;
                        for (auto const& dot : m_diagram->dot(string)) {
                            if (dot.dtype == type) {
                                hasThisType = true;
                                break;
                            }
                        }

                        if (hasThisType) {
                            continue;
                        }

                        dtype = type;
                        break;
                    }
                } else if (!m_automaticDotType) {
                    dtype = m_currentDtype;
                }

                // Ctrl adds a dot without removing other dots on a string
                m_diagram->undoSetFretDot(string, fret, haveCtrl, dtype);
            }
        }
    }
    m_diagram->triggerLayout();
    globalContext()->currentNotation()->undoStack()->commitChanges();
    update();

    globalContext()->currentNotation()->notationChanged().notify();
}

//---------------------------------------------------------
//   hoverMoveEvent
//---------------------------------------------------------

void FretCanvas::hoverMoveEvent(QHoverEvent* ev)
{
    int string;
    int fret;
    getPosition(ev->pos(), &string, &fret);
    if (m_cstring != string || m_cfret != fret) {
        m_cfret = fret;
        m_cstring = string;
        update();
    }
}

//---------------------------------------------------------
//   setFretDiagram
//---------------------------------------------------------

void FretCanvas::setFretDiagram(QVariant fd)
{
    m_diagram = fd.value<Ms::FretDiagram*>();

    emit diagramChanged(fd);

    if (m_diagram) {
        setImplicitHeight(m_diagram->frets() * 50);
    } else {
        setImplicitHeight(0);
        return;
    }

    update();
}

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void FretCanvas::clear()
{
    m_diagram->score()->startCmd();
    m_diagram->undoFretClear();
    m_diagram->score()->endCmd();
    update();
}

void FretCanvas::paint(QPainter* painter)
{
    draw(painter);
}

QVariant FretCanvas::diagram() const
{
    return QVariant::fromValue(m_diagram);
}

int FretCanvas::currentFretDotType() const
{
    return static_cast<int>(m_currentDtype);
}

bool FretCanvas::isBarreModeOn() const
{
    return m_barreMode;
}

bool FretCanvas::isMultipleDotsModeOn() const
{
    return m_multidotMode;
}

QColor FretCanvas::color() const
{
    return m_color;
}

void FretCanvas::setCurrentFretDotType(int currentFretDotType)
{
    Ms::FretDotType newDotType = static_cast<Ms::FretDotType>(currentFretDotType);

    if (m_currentDtype == newDotType) {
        return;
    }

    m_currentDtype = newDotType;
    emit currentFretDotTypeChanged(currentFretDotType);
}

void FretCanvas::setIsBarreModeOn(bool isBarreModeOn)
{
    if (m_barreMode == isBarreModeOn) {
        return;
    }

    m_barreMode = isBarreModeOn;
    emit isBarreModeOnChanged(m_barreMode);
}

void FretCanvas::setIsMultipleDotsModeOn(bool isMultipleDotsModeOn)
{
    if (m_multidotMode == isMultipleDotsModeOn) {
        return;
    }

    m_multidotMode = isMultipleDotsModeOn;
    emit isMultipleDotsModeOnChanged(m_multidotMode);
}

void FretCanvas::setColor(QColor color)
{
    if (m_color == color) {
        return;
    }

    m_color = color;
    update();
    emit colorChanged(m_color);
}
