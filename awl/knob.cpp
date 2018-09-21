//=============================================================================
//  Awl
//  Audio Widget Library
//  $Id:$
//
//  Copyright (C) 2002-2006 by Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "knob.h"

#include <QPen>

namespace Awl {

//---------------------------------------------------------
//   Knob
///   this is the AwlKnob constructor
//---------------------------------------------------------

Knob::Knob(QWidget* parent)
   : AbstractSlider(parent)
      {
      _spanDegrees = 270;
      _markSize  = 6;
      _border    = 2;
      points     = 0;
      }

//---------------------------------------------------------
//   Knob
//---------------------------------------------------------

Knob::~Knob()
      {
      if (points)
            delete points;
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void Knob::setText(const QString& s)
      {
      if (s != _text) {
            _text = s;
            update();
            }
      }

//---------------------------------------------------------
//   setScaleSize
//!   set the scale size in degrees
//
//!   the scale size is the max moving angle measured
//!   in degrees
//---------------------------------------------------------

void Knob::setSpanDegrees(double val)
      {
      if (val != _spanDegrees) {
            _spanDegrees = val;
            update();
            }
      }

//---------------------------------------------------------
//   setMarkSize
//!   set size of the center marker
//---------------------------------------------------------

void Knob::setMarkSize(int val)
      {
      if (val != _markSize) {
            _markSize = val;
            update();
            }
      }

//---------------------------------------------------------
//   setBorder
//!   set border size
//---------------------------------------------------------

void Knob::setBorder(int val)
      {
      if (val != _border) {
            _border = val;
            update();
            }
      }

//---------------------------------------------------------
//   setKnobIcon
//---------------------------------------------------------

void Knob::setKnobIcon(const QIcon& icon)
      {
      _knobIcon = icon;
      update();
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void Knob::mousePressEvent(QMouseEvent* ev)
      {
      startY = ev->y();
      emit sliderPressed(_id);
      if (_center) {
            QRect r(points->boundingRect().toRect());
            if (r.contains(ev->pos())) {
                  setValue(.0f);
                  valueChange();
                  }
            }
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void Knob::mouseReleaseEvent(QMouseEvent*)
      {
      emit sliderReleased(_id);
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void Knob::mouseMoveEvent(QMouseEvent* ev)
      {
      int y       = ev->y();
      double delta = (maxValue() - minValue()) / 100.0f;
      if (delta == 0)
            delta = 1;
      _value  += (startY - y) * delta;
      if (_value < minValue())
            _value = _minValue;
      else if (_value > maxValue())
            _value = _maxValue;
      startY    = y;
      valueChange();
      }

//---------------------------------------------------------
//   paintEvent
//    r - phys coord system
//---------------------------------------------------------

void Knob::paintEvent(QPaintEvent* /*ev*/)
      {
#if 0 // yet(?) unused
      QRect rr(ev->rect());
#endif
      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing, true);

      int markSize2  = _markSize/2;
      int emptyDegrees      = 360      - _spanDegrees;
      int w          = width()  - _scaleWidth - 2 * _border;
      int h          = height() - _scaleWidth/2 - 2 * _border;

      int xoffset, yoffset;
      if (_center)
            h -= _markSize;
      if (w > h) {
            yoffset = 0;
            xoffset = (w - h) / 2;
            w = h;
            }
      else {
            xoffset = 0;
            yoffset = h - w;              // top align
            h = w;
            }

      int x = xoffset + _scaleWidth / 2 + _border;
      int y = yoffset + _scaleWidth / 2 + _border + (_center ? _markSize+_scaleWidth/2 : 0);
      QRectF dialArea(x, y, w, h);

      QColor dialBgCol(isEnabled() ? _scaleColor : Qt::gray);
      QColor dialCol(isEnabled() ? _scaleValueColor : Qt::lightGray);

      //-----------------------------------------
      // draw arc
      //-----------------------------------------

      double maxVal = maxValue();
      double minVal = minValue();
      double val = value();

      double span = maxVal - minVal;

      p.setPen(QPen(dialBgCol, _scaleWidth));
      p.drawArc(dialArea, (90 + (_spanDegrees / 2)) * 16, -_spanDegrees * 16);

      if (_center) {
            double frac = (val - minVal) / span - 0.5;

            p.setPen(QPen(dialCol, _scaleWidth));
            p.drawArc(dialArea, 90 * 16, -frac * _spanDegrees * 16);
            }
      else {
            double frac = (val - minVal) / span;

            p.setPen(QPen(dialBgCol, _scaleWidth));
            p.drawArc(dialArea, (90 + (_spanDegrees / 2)) * 16, frac * -_spanDegrees * 16);
            }

      //-----------------------------------------
      // draw pointer
      //-----------------------------------------

      //knob image
      if (!_knobIcon.isNull()) {
            QRect r((int)dialArea.x(), (int)dialArea.y(), (int)dialArea.width(), (int)dialArea.height());
            _knobIcon.paint(&p, r);
            }

      //indicator line
      p.setPen(QPen(dialCol, _scaleWidth));
      double r1 = double(_spanDegrees) * (_value - _minValue) / span + 90.0
            + double(emptyDegrees / 2);
      r1     = r1 / 180.0 * M_PI;   // convert to radians
      int rd = w/2;
      int x1 = x + rd;
      int y1 = y + rd;
      int x2 = x1 + lrint(cos(r1) * double(rd));
      int y2 = y1 + lrint(sin(r1) * double(rd));
      p.drawLine(x1, y1, x2, y2);


      //-----------------------------------------
      // draw center mark
      //-----------------------------------------

      p.setPen(QPen(dialCol, 0));
      p.setBrush(dialCol);
      if (_center) {
            if (points)
                  delete points;
            qreal x3 = dialArea.width() / 2 + dialArea.x();
            qreal y3 = dialArea.y() - _markSize - _scaleWidth/2;
            points = new QPainterPath(QPointF(x3 - markSize2, y3));
            points->lineTo(x3 + markSize2, y3);
            points->lineTo(x3, _markSize + y3);
            points->closeSubpath();
            p.drawPath(*points);
            }

      //-----------------------------------------
      // draw text
      //-----------------------------------------

      if (!_text.isEmpty()) {
            p.drawText(dialArea, Qt::AlignBottom | Qt::AlignHCenter, _text);
            }
      }
}

