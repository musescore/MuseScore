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

#include "slider.h"

namespace Awl {

//---------------------------------------------------------
//   Slider
//---------------------------------------------------------

Slider::Slider(QWidget* parent)
   : AbstractSlider(parent), orient(Qt::Vertical), _sliderSize(14,14)
      {
      init();
      }

//---------------------------------------------------------
//   Slider
//---------------------------------------------------------

Slider::Slider(Qt::Orientation orientation, QWidget* parent)
   : AbstractSlider(parent), orient(orientation), _sliderSize(14,14)
      {
      init();
      }

//---------------------------------------------------------
//   Slider
//---------------------------------------------------------

void Slider::init()
      {
      _dclickValue1 = 0.0;
      _dclickValue2 = 0.0;
      if (orient == Qt::Vertical)
      	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
      else
      	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      dragMode = false;
      points  = 0;
      updateKnob();
      }

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize Slider::sizeHint() const
  	{
      int w = _sliderSize.width() + scaleWidth();
 	return orient == Qt::Vertical ? QSize(w, 200) : QSize(200, w);
      }

//---------------------------------------------------------
//   Slider
//---------------------------------------------------------

Slider::~Slider()
      {
      if (points)
      	delete points;
      }

//---------------------------------------------------------
//   setOrientation
//---------------------------------------------------------

void Slider::setOrientation(Qt::Orientation o)
      {
      orient = o;
      updateKnob();
      update();
      }

//---------------------------------------------------------
//   updateKnob
//---------------------------------------------------------

void Slider::updateKnob()
      {
	if (points)
      	delete points;
	points = new QPainterPath;
      int kh  = _sliderSize.height();
    	int kw  = _sliderSize.width();
      points->moveTo(0.0, 0.0);
      if (orient == Qt::Vertical) {
      	int kh  = _sliderSize.height();
            int kh2  = kh / 2;
            points->lineTo(kw, -kh2);
            points->lineTo(kw, kh2);
            }
      else {
            int kw2 = kw/2;
            points->lineTo(-kw2, kh);
            points->lineTo(kw2, kh);
            }
      points->lineTo(0.0, 0.0);
      }

//---------------------------------------------------------
//   setInvertedAppearance
//---------------------------------------------------------

void Slider::setInvertedAppearance(bool val)
      {
      AbstractSlider::setInvertedAppearance(val);
      update();
      }

//---------------------------------------------------------
//   setSliderSize
//---------------------------------------------------------

void Slider::setSliderSize(const QSize& s)
      {
      _sliderSize = s;
      update();
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void Slider::mousePressEvent(QMouseEvent* ev)
      {
      startDrag = ev->pos();
//      if (points->boundingRect().toRect().contains(startDrag)) {
            emit sliderPressed(_id);
            dragMode = true;
            int pixel = (orient == Qt::Vertical) ? height() - _sliderSize.height() : width() - _sliderSize.width();
            dragppos = int(pixel * (_value - minValue()) / (maxValue() - minValue()));
            if (_invert)
                  dragppos = pixel - dragppos;
//            }
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void Slider::mouseReleaseEvent(QMouseEvent*)
      {
      if (dragMode) {
            emit sliderReleased(_id);
            dragMode = false;
            }
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void Slider::mouseMoveEvent(QMouseEvent* ev)
      {
      if (!dragMode)
            return;
      int delta = orient == Qt::Horizontal ? (startDrag.x() - ev->x()) : (startDrag.y() - ev->y());

//      if (_invert)
//            delta = -delta;
      if (orient == Qt::Horizontal)
            delta = -delta;
      int ppos = dragppos + delta;
      if (ppos < 0)
            ppos = 0;

      int pixel = (orient == Qt::Vertical) ? height() - _sliderSize.height() : width() - _sliderSize.width();
      if (ppos > pixel)
            ppos = pixel;
      int pos = _invert ? (pixel - ppos) : ppos;
      _value  = (pos * (maxValue() - minValue()) / pixel) + minValue() - 0.000001;
      valueChange();
      }

//---------------------------------------------------------
//   paint
//    r - phys coord system
//---------------------------------------------------------

void Slider::paintEvent(QPaintEvent* /*ev*/)
      {
      int h   = height();
      int w   = width();
      int kw  = _sliderSize.width();
      int kh  = _sliderSize.height();
      int pixel = (orient == Qt::Vertical) ? h - kh : w - kw;
      double range = maxValue() - minValue();
      int ppos = int(pixel * (_value - minValue()) / range);
      if ((orient == Qt::Vertical && _invert) || (orient == Qt::Horizontal && !_invert))
            ppos = pixel - ppos;

#if 0 // yet(?) unused
      QRect rr(ev->rect());
#endif
      QPainter p(this);

      QColor sc(isEnabled() ? _scaleColor : Qt::gray);
      QColor svc(isEnabled() ? _scaleValueColor : Qt::gray);
      p.setBrush(svc);

      int kh2 = kh/2;

      //---------------------------------------------------
      //    draw scale
      //---------------------------------------------------

      if (orient == Qt::Vertical) {
            int xm = (w - _scaleWidth - _sliderSize.height()) / 2;
      	int y1 = kh2;
      	int y2 = h - (ppos + y1);
      	int y3 = h - y1;
            p.fillRect(xm, y1, _scaleWidth, y2-y1, _invert ? svc : sc);
            p.fillRect(xm, y2, _scaleWidth, y3-y2, _invert ? sc : svc);
      	p.translate(QPointF(xm + _scaleWidth/2, y2));
            }
      else {
            int ym = (h - _scaleWidth - _sliderSize.height()) / 2;
      	int x1 = kh2;
      	int x2 = w - (ppos + x1);
      	int x3 = w - x1;
            p.fillRect(x1, ym, x2-x1, _scaleWidth, _invert ? sc : svc);
            p.fillRect(x2, ym, x3-x2, _scaleWidth, _invert ? svc : sc);
      	p.translate(QPointF(x2, ym + _scaleWidth/2));
            }

      //---------------------------------------------------
      //    draw slider
      //---------------------------------------------------

  	p.setRenderHint(QPainter::Antialiasing, true);
	p.setPen(QPen(svc, 0));
     	p.drawPath(*points);
      }
}

