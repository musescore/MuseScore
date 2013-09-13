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

#include "fastlog.h"
#include "aslider.h"

namespace Awl {

//---------------------------------------------------------
//   AbstractSlider
//---------------------------------------------------------

AbstractSlider::AbstractSlider(QWidget* parent)
   : QWidget(parent), _scaleColor(Qt::darkGray), _scaleValueColor(QColor("#2456aa"))
      {
      _id         = 0;
      _value      = 0.5;
      _minValue   = 0.0;
      _maxValue   = 1.0;
      _lineStep   = 0.1;
      _pageStep   = 0.2;
      _center     = false;
      _invert     = false;
      _scaleWidth = 4;
      _log        = false;
      setFocusPolicy(Qt::StrongFocus);
      }

//---------------------------------------------------------
//   setEnabled
//---------------------------------------------------------

void AbstractSlider::setEnabled(bool val)
      {
      QWidget::setEnabled(val);
      update();
      }

//---------------------------------------------------------
//   setCenter
//!   If the center flag is set, a notch is drawn to
//!   show the center position.
//---------------------------------------------------------

void AbstractSlider::setCenter(bool val)
      {
      if (val != _center) {
            _center = val;
            update();
            }
      }

//!--------------------------------------------------------
//   setScaleWidth
//---------------------------------------------------------

void AbstractSlider::setScaleWidth(int val)
      {
      if (val != _scaleWidth) {
            _scaleWidth = val;
            update();
            }
      }

//---------------------------------------------------------
//   setScaleColor
//---------------------------------------------------------

void AbstractSlider::setScaleColor(const QColor& c)
      {
      if (c != _scaleColor) {
            _scaleColor = c;
            update();
            }
      }

//---------------------------------------------------------
//   setScaleValueColor
//---------------------------------------------------------

void AbstractSlider::setScaleValueColor(const QColor& c)
      {
      if (c != _scaleValueColor) {
            _scaleValueColor = c;
            update();
            }
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void AbstractSlider::wheelEvent(QWheelEvent* ev)
      {
      int div = 50;
      if (ev->modifiers() & Qt::ShiftModifier)
            div = 15;
      _value += (ev->delta() * lineStep()) / div;
      if (_value < _minValue)
            _value = _minValue;
      else if (_value > _maxValue)
            _value = _maxValue;
      valueChange();
      }

//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

void AbstractSlider::keyPressEvent(QKeyEvent* ev)
      {
      double oval = _value;

      switch (ev->key()) {
            case Qt::Key_Home:    _value = _minValue; break;
            case Qt::Key_End:     _value = _maxValue; break;
            case Qt::Key_Up:
            case Qt::Key_Left:    _value += lineStep(); break;
            case Qt::Key_Down:
            case Qt::Key_Right:   _value -= lineStep(); break;
            case Qt::Key_PageDown: _value -= pageStep(); break;
            case Qt::Key_PageUp:   _value += pageStep(); break;
            default:
                  break;
            }
      if (_value < _minValue)
            _value = _minValue;
      else if (_value > _maxValue)
            _value = _maxValue;
      if (oval != _value)
            valueChange();
      }

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void AbstractSlider::setValue(double val)
      {
      if (_log) {
            if (val == 0.0f)
                  _value = _minValue;
            else {
                  _value = fast_log10(val) * 20.0f;
       		if (_value < _minValue)
            		_value = _minValue;
                 	}
            }
      else
            _value = val;
      update();
      }

//---------------------------------------------------------
//   valueChange
//---------------------------------------------------------

void AbstractSlider::valueChange()
      {
      emit valueChanged(value(), _id);
      update();
      }

//---------------------------------------------------------
//   value
//---------------------------------------------------------

double AbstractSlider::value() const
      {
      return _log ? pow(10.0, _value*0.05f) : _value;
      }

//---------------------------------------------------------
//   minLogValue
//---------------------------------------------------------

//double AbstractSlider::minValue() const {
//  return _log ? pow(10.0, _minValue*0.05f) : _minValue;
//}

//---------------------------------------------------------
//   setMinLogValue
//---------------------------------------------------------

void AbstractSlider::setMinLogValue(double val)
      {
      if (_log) {
            if (val == 0.0f)
                  _minValue = -100;
            else
                  _minValue = fast_log10(val) * 20.0f;
            }
      else
            _minValue = val;
      }

//---------------------------------------------------------
//   maxLogValue
//---------------------------------------------------------

//double AbstractSlider::maxValue() const {
//  return _log ? pow(10.0, _maxValue*0.05f) : _maxValue;
//}

//---------------------------------------------------------
//   setMaxLogValue
//---------------------------------------------------------

void AbstractSlider::setMaxLogValue(double val)
      {
      if (_log) {
            _maxValue = fast_log10(val) * 20.0f;
            }
      else
            _maxValue = val;
      }

#if 0
//---------------------------------------------------------
//   init
//---------------------------------------------------------

void AbstractSlider::init(const SyntiParameter& f)
      {
      _minValue = f.min();
      _maxValue = f.max();
      _value    = f.fval();
      _lineStep   = (_maxValue - _minValue) * 0.1;
      _pageStep   = _lineStep * 2.0;
      }
#endif
}
