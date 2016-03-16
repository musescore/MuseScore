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

namespace Awl {

//---------------------------------------------------------
//   Knob
///   this is the AwlKnob contructor
//---------------------------------------------------------

Knob::Knob(QWidget* parent)
   : AbstractSlider(parent)
      {
      _scaleSize = 270;
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

void Knob::setScaleSize(int val)
      {
      if (val != _scaleSize) {
            _scaleSize = val;
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
      int restR      = 360      - _scaleSize;
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
            // yoffset = (h - w) / 2;     // center
            yoffset = h - w;              // top align
            h = w;
            }

      int x = xoffset + _scaleWidth / 2 + _border;
      int y = yoffset + _scaleWidth / 2 + _border + (_center ? _markSize+_scaleWidth/2 : 0);
      QRectF ar(x, y, w, h);

      QColor sc(isEnabled() ? _scaleColor : Qt::gray);
      QColor svc(isEnabled() ? _scaleValueColor : Qt::gray);

      //-----------------------------------------
      // draw arc
      //-----------------------------------------

      double dvalue = maxValue() - minValue();
      if (_center) {
            int size = _scaleSize * 8;
            if (_value >= 0) {
                  int offset = (restR-180) * 8;
                  int r1 = int (size * _value / maxValue());
                  int r2 = size - r1;
                  p.setPen(QPen(sc, _scaleWidth));
                  if (r2 > 1)
                        p.drawArc(ar, offset, r2);
                  if (size > 1)
                        p.drawArc(ar, 90*16, size);
                  if (r1 > 1) {
                        p.setPen(QPen(svc, _scaleWidth));
                        p.drawArc(ar, offset+r2, r1);
                        }
                  }
            else {
                  // int offset = (restR+180) * 8;
                  int r1 = int(size * _value / minValue());
                  int r2 = size - r1;

                  p.setPen(QPen(sc, _scaleWidth));
                  if (size > 1)
                        p.drawArc(ar, (restR-180)*8, size);
                  if (r2 > 1)
                        p.drawArc(ar, 90 * 16 + r1, r2);
                  if (r1 > 1) {
                        p.setPen(QPen(svc, _scaleWidth));
                        p.drawArc(ar, 90*16, r1);
                        }
                  }
            }
      else {
            int offset = (180-restR) * 8;
            int size   = _scaleSize * 16;
            int r1     = int(size * (_value - minValue()) / dvalue);
            int r2     = size - r1;
            if (r2 >= 1) {
                  p.setPen(QPen(sc, _scaleWidth));
                  p.drawArc(ar, -offset, r2);
                  }
            if (r1 >= 1) {
                  p.setPen(QPen(svc, _scaleWidth));
                  p.drawArc(ar, r2-offset, r1);
                  }
            }

      //-----------------------------------------
      // draw pointer
      //-----------------------------------------

      p.setPen(QPen(svc, _scaleWidth));
      double r1 = double(_scaleSize) * (_value-minValue()) / dvalue + 90.0
            + double(restR/2);
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

      p.setPen(QPen(svc, 0));
      p.setBrush(svc);
      if (_center) {
            if (points)
                  delete points;
            qreal x = ar.width() / 2 + ar.x();
            qreal y = ar.y() - _markSize - _scaleWidth/2;
            points = new QPainterPath(QPointF(x - markSize2, y));
            points->lineTo(x + markSize2, y);
            points->lineTo(x, _markSize + y);
            points->closeSubpath();
            p.drawPath(*points);
            }

      //-----------------------------------------
      // draw text
      //-----------------------------------------

      if (!_text.isEmpty()) {
            p.drawText(ar, Qt::AlignBottom | Qt::AlignHCenter, _text);
            }
      }
}

