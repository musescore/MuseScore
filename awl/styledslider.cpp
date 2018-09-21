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

#include "styledslider.h"

#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionSlider>
#include <QMouseEvent>
#include <QDebug>

namespace Awl {

//---------------------------------------------------------
//   StyledSlider
//---------------------------------------------------------

StyledSlider::StyledSlider(QWidget *parent) : QWidget(parent)
      {
      setFocusPolicy(Qt::StrongFocus);
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void StyledSlider::wheelEvent(QWheelEvent* e)
      {
      QPoint ad = e->angleDelta();
      setValue(_value + (_maxValue - _minValue) * ad.y() / 10000.0);
      repaint();
      }

//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

void StyledSlider::keyPressEvent(QKeyEvent* e)
      {
      switch (e->key())
            {
            case Qt::Key_Up:
                  setValue(_value + 1);
                  break;
            case Qt::Key_Down:
                  setValue(_value - 1);
                  break;
            }
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void StyledSlider::mousePressEvent(QMouseEvent* e)
      {
      draggingMouse = true;
      mouseDownPos = e->pos();
      lastMousePos = mouseDownPos;
      emit(sliderPressed());
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void StyledSlider::mouseReleaseEvent(QMouseEvent*)
      {
      draggingMouse = false;

      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void StyledSlider::mouseMoveEvent(QMouseEvent* e)
      {
      if (draggingMouse) {
            QPoint p = e->pos();
            double dPixY = p.y() - lastMousePos.y();
            double barLength = height() - (_margin * 2);
            double dy = dPixY * (_maxValue - _minValue) / barLength;

            double val = qBound(_minValue, _value - dy, _maxValue);

            lastMousePos = p;
            setValue(val);
            repaint();
            }
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void StyledSlider::paintEvent(QPaintEvent *ev)
{
    QWidget::paintEvent(ev);

    QPainter p(this);
    //QRect geo = this->geometry();

    int w = width();
    int h = height();

    double barLength = h - _margin * 2;
    double x0 = (w - _barThickness) / 2;
    double valueSpan = _maxValue - _minValue;
    double span = (_value - _minValue) / (_maxValue - _minValue);
    span = qMin(qMax(span, 0.0), 1.0);
    double y1 = (1 - span) * barLength;

    double midPtPix = w / 2.0;

    //Ticks
    p.setPen(QPen(_tickColor));

    for (qreal yVal = _minValue; yVal <= _maxValue + .00001; yVal += _minorTickSpacing) {
          qreal yPix = (yVal - _minValue) / valueSpan;
          yPix = (1.0 - yPix) * barLength + _margin;
          QLineF line(midPtPix - _minorTickWidth, yPix, midPtPix + _minorTickWidth, yPix);
          p.drawLine(line);
          }

    for (qreal yVal = _minValue; yVal <= _maxValue + .00001; yVal += _majorTickSpacing) {
          qreal yPix = (yVal - _minValue) / valueSpan;
          yPix = (1.0 - yPix) * barLength + _margin;
          QLineF line(midPtPix - _majorTickWidth, yPix, midPtPix + _majorTickWidth, yPix);
          p.drawLine(line);
          }


    //Bars
    double yPix = _margin + y1;
    QRectF bgRect(x0, _margin, _barThickness, barLength);
    QRectF hiliteRect(x0, yPix, _barThickness, barLength - y1);

    p.fillRect(bgRect, QBrush(_backgroundColor));

    p.fillRect(hiliteRect, QBrush(_hilightColor));

    //Slider head
    if (!_sliderHeadIcon.isNull()) {
          QRect r((int)midPtPix - 14, (int)yPix - 20, (int)28, (int)40);
          _sliderHeadIcon.paint(&p, r);
          }
}

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void StyledSlider::setValue(double v)
      {
      if (v == _value)
            return;
//      qDebug() << "Value changed v:" << v;

      _value = v;
      repaint();
      emit(valueChanged(v));
      }

//---------------------------------------------------------
//   setMinValue
//---------------------------------------------------------

void StyledSlider::setMinValue(double v)
      {
      if (v == _minValue)
            return;
      _minValue = v;
      repaint();
      emit(minValueChanged(v));
      }

//---------------------------------------------------------
//   setMaxValue
//---------------------------------------------------------

void StyledSlider::setMaxValue(double v)
      {
      if (v == _maxValue)
            return;
      _maxValue = v;
      repaint();
      emit(maxValueChanged(v));
      }

//---------------------------------------------------------
//   setBackgroundColor
//---------------------------------------------------------

void StyledSlider::setBackgroundColor(QColor v)
      {
      _backgroundColor = v;
      repaint();
      }

//---------------------------------------------------------
//   setHilightColor
//---------------------------------------------------------

void StyledSlider::setHilightColor(QColor v)
      {
      _hilightColor = v;
      repaint();
      }

//---------------------------------------------------------
//   setTickColor
//---------------------------------------------------------

void StyledSlider::setTickColor(QColor v)
      {
      _tickColor = v;
      repaint();
      }

//---------------------------------------------------------
//   setBarThickness
//---------------------------------------------------------

void StyledSlider::setBarThickness(double v)
      {
      _barThickness = v;
      repaint();
      }

//---------------------------------------------------------
//   setMargin
//---------------------------------------------------------

void StyledSlider::setMargin(double v)
      {
      _margin = v;
      repaint();
      }

//---------------------------------------------------------
//   setMajorTickSpacing
//---------------------------------------------------------

void StyledSlider::setMajorTickSpacing(double v)
      {
      _majorTickSpacing = v;
      repaint();
      }

//---------------------------------------------------------
//   setMinorTickSpacing
//---------------------------------------------------------

void StyledSlider::setMinorTickSpacing(double v)
      {
      _minorTickSpacing = v;
      repaint();
      }

//---------------------------------------------------------
//   setMajorTickWidth
//---------------------------------------------------------

void StyledSlider::setMajorTickWidth(double v)
      {
      _majorTickWidth = v;
      repaint();
      }

//---------------------------------------------------------
//   setMinorTickWidth
//---------------------------------------------------------

void StyledSlider::setMinorTickWidth(double v)
      {
      _minorTickWidth = v;
      repaint();
      }

//---------------------------------------------------------
//   setSliderHeadIcon
//---------------------------------------------------------

void StyledSlider::setSliderHeadIcon(QIcon v)
      {
      _sliderHeadIcon = v;
      repaint();
      }

}
