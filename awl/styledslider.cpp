//=============================================================================
//  Awl
//  Audio Widget Library
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



void StyledSlider::mouseDoubleClickEvent(QMouseEvent*)
      {
      setValue(_doubleClickValue);
      }



//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void StyledSlider::wheelEvent(QWheelEvent* e)
      {
      QPoint ad = e->angleDelta();
      //120 degrees is one tick
      auto value = _value + ad.y() / 120;
      value = qBound(_minValue, value, _maxValue);
      setValue(value);
      update();
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
      mouseDownVal = _value;
      emit sliderPressed();
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
            double dPixY = p.y() - mouseDownPos.y();
            double barLength = height() - (_margin * 2);
            double dy = dPixY * (_maxValue - _minValue) / barLength;

            double val = qBound(_minValue, mouseDownVal - dy, _maxValue);

            setValue(val);
            update();
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
//    double valueSpan = _maxValue - _minValue;
    double span = (_value - _minValue) / (_maxValue - _minValue);
    span = qMin(qMax(span, 0.0), 1.0);
    double y1 = (1 - span) * barLength;

    double midPtPix = w / 2.0;

    //Ticks
    p.setPen(QPen(_tickColor));

//    for (qreal yVal = _minValue; yVal <= _maxValue + .00001; yVal += _numMinorTicks) {
//          qreal yPix = (yVal - _minValue) / valueSpan;
//          yPix = (1.0 - yPix) * barLength + _margin;
//          QLineF line(midPtPix - _minorTickWidth, yPix, midPtPix + _minorTickWidth, yPix);
//          p.drawLine(line);
//          }

//    for (qreal yVal = _minValue; yVal <= _maxValue + .00001; yVal += _numMajorTicks) {
//          qreal yPix = (yVal - _minValue) / valueSpan;
//          yPix = (1.0 - yPix) * barLength + _margin;
//          QLineF line(midPtPix - _majorTickWidth, yPix, midPtPix + _majorTickWidth, yPix);
//          p.drawLine(line);

//          for (int i = 1; i < _numMinorTicks; ++i) {
//              qreal yyPix = yPix + (i * _numMajorTicks) / _numMinorTicks;
//              QLineF line(midPtPix - _minorTickWidth, yyPix, midPtPix + _minorTickWidth, yyPix);
//              p.drawLine(line);
//              }

//          }

    for (int i = 0; i <= _numMajorTicks; ++i) {
          qreal yVal = i * barLength / _numMajorTicks + _margin;
          QLineF line(midPtPix - _majorTickWidth, yVal, midPtPix + _majorTickWidth, yVal);
          p.drawLine(line);

          if (i < _numMajorTicks) {
                qreal yValNext = (i + 1) * barLength / _numMajorTicks + _margin;
                for (int j = 1; j < _numMinorTicks; ++j) {
                      qreal yyVal = yVal + j * (yValNext - yVal) / _numMinorTicks;

                      QLineF line1(midPtPix - _minorTickWidth, yyVal, midPtPix + _minorTickWidth, yyVal);
                      p.drawLine(line1);
                  }
              }
          }

    //Marks
//    p.setPen(QPen(_tickColor, 2));

//    for (double mark: marks) {
//        qreal yPix = (mark - _minValue) / valueSpan;
//        yPix = (1.0 - yPix) * barLength + _margin;

//        QLineF line(midPtPix - _majorTickWidth, yPix, midPtPix + _majorTickWidth, yPix);
//        p.drawLine(line);
//        }


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

      _value = v;
      update();
      emit valueChanged(v);
      }

//---------------------------------------------------------
//   setMinValue
//---------------------------------------------------------

void StyledSlider::setMinValue(double v)
      {
      if (v == _minValue)
            return;
      _minValue = v;
      update();
      emit minValueChanged(v);
      }

//---------------------------------------------------------
//   setMaxValue
//---------------------------------------------------------

void StyledSlider::setMaxValue(double v)
      {
      if (v == _maxValue)
            return;
      _maxValue = v;
      update();
      emit maxValueChanged(v);
      }

//---------------------------------------------------------
//   setBackgroundColor
//---------------------------------------------------------

void StyledSlider::setBackgroundColor(QColor v)
      {
      _backgroundColor = v;
      update();
      }

//---------------------------------------------------------
//   setHilightColor
//---------------------------------------------------------

void StyledSlider::setHilightColor(QColor v)
      {
      _hilightColor = v;
      update();
      }

//---------------------------------------------------------
//   setTickColor
//---------------------------------------------------------

void StyledSlider::setTickColor(QColor v)
      {
      _tickColor = v;
      update();
      }

//---------------------------------------------------------
//   setBarThickness
//---------------------------------------------------------

void StyledSlider::setBarThickness(double v)
      {
      _barThickness = v;
      update();
      }

//---------------------------------------------------------
//   setMargin
//---------------------------------------------------------

void StyledSlider::setMargin(double v)
      {
      _margin = v;
      update();
      }

//---------------------------------------------------------
//   setMajorTickSpacing
//---------------------------------------------------------

void StyledSlider::setNumMajorTicks(int v)
      {
      _numMajorTicks = v;
      update();
      }

//---------------------------------------------------------
//   setMinorTickSpacing
//---------------------------------------------------------

void StyledSlider::setNumMinorTicks(int v)
      {
      _numMinorTicks = v;
      update();
      }

//---------------------------------------------------------
//   setMajorTickWidth
//---------------------------------------------------------

void StyledSlider::setMajorTickWidth(double v)
      {
      _majorTickWidth = v;
      update();
      }

//---------------------------------------------------------
//   setMinorTickWidth
//---------------------------------------------------------

void StyledSlider::setMinorTickWidth(double v)
      {
      _minorTickWidth = v;
      update();
      }

//---------------------------------------------------------
//   setSliderHeadIcon
//---------------------------------------------------------

void StyledSlider::setSliderHeadIcon(QIcon v)
      {
      _sliderHeadIcon = v;
      update();
      }

}
