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

#include "fretcanvas.h"

#include <cmath>

#include "engraving/dom/fret.h"

using namespace mu::inspector;

FretCanvas::FretCanvas(QQuickItem* parent)
    : muse::uicomponents::QuickPaintedView(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);

    m_cstring = -2;
    m_cfret   = -2;
}

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
    double yNut = -0.5 * (lw2 - lw1);
    painter->drawLine(QLineF(-lw1 * .5, yNut, x2 + lw1 * .5, yNut));

    pen.setWidthF(lw1);
    painter->setPen(pen);
    double y2 = _frets * fretDist + 0.5 * lw1;

    QPen symPen(pen);
    symPen.setCapStyle(Qt::RoundCap);
    symPen.setWidthF(lw1 * 1.2);

    // Draw strings and frets
    for (int i = 0; i < _strings; ++i) {
        double x = stringDist * i;
        painter->drawLine(QLineF(x, 0.0, x, y2));
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

        mu::engraving::FretItem::Marker mark = m_diagram->marker(i);
        if (mark.exists()) {
            painter->setFont(font);
            double x = stringDist * i;
            double y = -fretDist * .1;
            painter->drawText(QRectF(x, y, 0.0, 0.0),
                              Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip,
                              QChar(mu::engraving::FretItem::markerToChar(mark.mtype).unicode()));
        }
    }

    // Draw barres
    painter->setPen(pen);
    for (auto const& i : m_diagram->barres()) {
        int fret        = i.first;
        int startString = i.second.startString;
        int endString   = i.second.endString;

        if (m_diagram->score()->style().styleB(mu::engraving::Sid::barreAppearanceSlur)) {
            // Copy-pasted from TLayout and TDraw, like most of the other code in this class.
            // This probably needs a better solution in the future. (M.S.)
            double insetX = 2 * lw1;
            double insetY = fret == 1 ? lw2 + lw1 : insetX;
            double startX = startString * stringDist + insetX;
            double endX = (endString == -1 ? x2 : stringDist * endString) - insetX;
            double shoulderXoffset = 0.2 * (endX - startX);
            double startEndY = (fret - 1) * fretDist - insetY;
            double shoulderY = startEndY - 0.5 * fretDist;
            double slurThickness = 0.1 * _spatium;
            double shoulderYfor = shoulderY - slurThickness;
            double shoulderYback = shoulderY + slurThickness;
            QPointF bezier1for = QPointF(startX + shoulderXoffset, shoulderYfor);
            QPointF bezier2for = QPointF(endX - shoulderXoffset, shoulderYfor);
            QPointF bezier1back = QPointF(startX + shoulderXoffset, shoulderYback);
            QPointF bezier2back = QPointF(endX - shoulderXoffset, shoulderYback);
            QPainterPath slurPath = QPainterPath();
            slurPath.moveTo(startX, startEndY);
            slurPath.cubicTo(bezier1for, bezier2for, QPointF(endX, startEndY));
            slurPath.cubicTo(bezier2back, bezier1back, QPointF(startX, startEndY));
            pen.setWidthF(0.25 * lw1);
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::RoundJoin);
            painter->setPen(pen);
            painter->setBrush(pen.color());
            painter->drawPath(slurPath);
        } else {
            qreal x1   = stringDist * startString;
            qreal newX2 = endString == -1 ? x2 : stringDist * endString;

            qreal y    = fretDist * (fret - 1) + fretDist * .5;
            pen.setWidthF(dotd * m_diagram->score()->style().styleD(mu::engraving::Sid::barreLineWidth));      // don't use style barreLineWidth - why not?
            pen.setCapStyle(Qt::RoundCap);
            painter->setPen(pen);
            painter->drawLine(QLineF(x1, y, newX2, y));
        }
    }

    // Draw 'hover' dot
    if ((m_cfret > 0) && (m_cfret <= _frets) && (m_cstring >= 0) && (m_cstring < _strings)) {
        mu::engraving::FretItem::Dot cd = m_diagram->dot(m_cstring, m_cfret)[0];
        std::vector<mu::engraving::FretItem::Dot> otherDots = m_diagram->dot(m_cstring);
        mu::engraving::FretDotType dtype;
        symPen.setColor(Qt::lightGray);

        if (cd.exists()) {
            dtype = cd.dtype;
            symPen.setColor(Qt::red);
        } else {
            dtype = m_automaticDotType ? mu::engraving::FretDotType::NORMAL : m_currentDtype;
        }
        painter->setPen(symPen);

        double x = stringDist * m_cstring - dotd * .5;
        double y = fretDist * (m_cfret - 1) + fretDist * .5 - dotd * .5;
        painter->setBrush(Qt::lightGray);
        paintDotSymbol(painter, symPen, x, y, dotd, dtype);
    }

    if (fretOffset > 0) {  // TODO: get the value from Sid::fretNumMag
        painter->setFont(font);
        painter->setPen(pen);
        // Todo: make dependent from Sid::fretNumPos
        painter->drawText(QRectF(-stringDist * .4, 0.0, 0.0, fretDist),
                          Qt::AlignVCenter | Qt::AlignRight | Qt::TextDontClip,
                          QString("%1").arg(fretOffset + 1));
    }

    if (m_diagram->showFingering()) {
        // Copy-pasted from layout and drawing code. Needs cleanup in future. (M.S.)
        const double padding = 0.2 * _spatium;
        muse::draw::FontMetrics fontMetrics(muse::draw::Font::fromQFont(font, muse::draw::Font::Type::Text));
        double fontHeight = fontMetrics.capHeight();
        for (size_t i = 0; i < m_diagram->fingering().size(); ++i) {
            int finger = m_diagram->fingering()[i];
            if (finger == 0) {
                continue;
            }
            QString fingerS = QString::number(finger);
            double width = fontMetrics.width(fingerS);
            double xOff = -0.5 * width;
            double fingerX = (m_diagram->strings() - i - 1) * stringDist + xOff;
            double fingerY = (m_diagram->frets() * fretDist) + fontHeight + padding;
            painter->setPen(pen);
            painter->setFont(font);
            painter->drawText(QPointF(fingerX, fingerY), fingerS);
        }
    }
}

void FretCanvas::paintDotSymbol(QPainter* p, QPen& pen, qreal x, qreal y, qreal dotd, mu::engraving::FretDotType dtype)
{
    switch (dtype) {
    case mu::engraving::FretDotType::CROSS:
        p->drawLine(QLineF(x, y, x + dotd, y + dotd));
        p->drawLine(QLineF(x + dotd, y, x, y + dotd));
        break;
    case mu::engraving::FretDotType::SQUARE:
        p->setBrush(Qt::NoBrush);
        p->drawRect(QRectF(x, y, dotd, dotd));
        break;
    case mu::engraving::FretDotType::TRIANGLE:
        p->drawLine(QLineF(x, y + dotd, x + .5 * dotd, y));
        p->drawLine(QLineF(x + .5 * dotd, y, x + dotd, y + dotd));
        p->drawLine(QLineF(x + dotd, y + dotd, x, y + dotd));
        break;
    case mu::engraving::FretDotType::NORMAL:
    default:
        p->setBrush(pen.color());
        p->setPen(Qt::NoPen);
        p->drawEllipse(QRectF(x, y, dotd, dotd));
        break;
    }
}

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

    globalContext()->currentNotation()->undoStack()->prepareChanges(muse::TranslatableString("undoableAction", "Edit fretboard diagram"));

    // Click above the fret diagram, so change the open/closed string marker
    if (fret == 0) {
        switch (m_diagram->marker(string).mtype) {
        case mu::engraving::FretMarkerType::CIRCLE:
            m_diagram->undoSetFretMarker(string, mu::engraving::FretMarkerType::CROSS);
            break;
        case mu::engraving::FretMarkerType::CROSS:
            m_diagram->undoSetFretMarker(string, mu::engraving::FretMarkerType::NONE);
            break;
        case mu::engraving::FretMarkerType::NONE:
        default:
            m_diagram->undoSetFretDot(string, 0);
            m_diagram->undoSetFretMarker(string, mu::engraving::FretMarkerType::CIRCLE);
            break;
        }
    }
    // Otherwise, the click is on the fretboard itself
    else {
        mu::engraving::FretItem::Dot thisDot = m_diagram->dot(string, fret)[0];
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
                mu::engraving::FretDotType dtype = mu::engraving::FretDotType::NORMAL;
                if (m_automaticDotType && haveCtrl && m_diagram->dot(string)[0].exists()) {
                    dtype = mu::engraving::FretDotType::TRIANGLE;

                    std::vector<mu::engraving::FretDotType> dtypes {
                        mu::engraving::FretDotType::NORMAL,
                        mu::engraving::FretDotType::CROSS,
                        mu::engraving::FretDotType::SQUARE,
                        mu::engraving::FretDotType::TRIANGLE
                    };

                    // Find the lowest dot type that doesn't already exist on the string
                    for (int i = 0; i < int(dtypes.size()); i++) {
                        mu::engraving::FretDotType type = dtypes[i];

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

void FretCanvas::hoverMoveEvent(QHoverEvent* ev)
{
    int string = 0;
    int fret = 0;

    getPosition(ev->position(), &string, &fret);
    if (m_cstring != string || m_cfret != fret) {
        m_cfret = fret;
        m_cstring = string;
        update();
    }
}

void FretCanvas::setFretDiagram(QVariant fd)
{
    m_diagram = fd.value<mu::engraving::FretDiagram*>();

    emit diagramChanged(fd);

    if (m_diagram) {
        setImplicitHeight(m_diagram->frets() * 50);
    } else {
        setImplicitHeight(0);
        return;
    }

    update();
}

void FretCanvas::clear()
{
    globalContext()->currentNotation()->undoStack()->prepareChanges(muse::TranslatableString("undoableAction", "Clear fretboard diagram"));
    m_diagram->undoFretClear();
    globalContext()->currentNotation()->undoStack()->commitChanges();
    update();

    globalContext()->currentNotation()->notationChanged().notify();
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
    mu::engraving::FretDotType newDotType = static_cast<mu::engraving::FretDotType>(currentFretDotType);

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
