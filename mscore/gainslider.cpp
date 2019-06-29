//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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

#include "gainslider.h"

namespace Ms {
#define MIXER_VOLUME_SLIDER_STEPS 80
GainSlider::GainSlider(QWidget* parent) : QSlider(parent)
{
      setMinimum(-60);
      setMaximum(20);
      setLogRange(0.0f, 10.0f);
}

void GainSlider::setMinLogValue(double val)
{
      if (val == 0.0f)
            _minValue = -100;
      else
            _minValue = fast_log10(val) * 20.0f;
}

void GainSlider::setMaxLogValue(double val)
{
      _maxValue = fast_log10(val) * 20.0f;
}

void GainSlider::setDoubleValue(double newValue)
{
      double positionValue;

      if (newValue == 0.0f)
            positionValue = _minValue;
      else {
            positionValue = fast_log10(newValue) * 20.0f;
            if (positionValue < _minValue)
                  positionValue = _minValue;
      }

      // Update the position. This is for the case when the
      // method is externally rather by the call in sliderChange()
      if (value() != int(positionValue)) {
            QSlider::setValue(int(positionValue + 0.5));
            emit doubleValueChanged(newValue);
            }
}

double GainSlider::doubleValue() const
      {
      return pow(10.0, double(value())*0.05f);
      }


void GainSlider::sliderChange(QAbstractSlider::SliderChange change)
      {
      if (change == QAbstractSlider::SliderValueChange)  {
            double newPositionValue = double(value());
            double newDoubleValue = pow(10.0, newPositionValue*0.05f);
            setDoubleValue(newDoubleValue);
      }
      QSlider::sliderChange(change);
      }

}
