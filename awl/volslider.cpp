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

#include "fastlog.h"
#include "volslider.h"

namespace Awl {

//---------------------------------------------------------
//   VolSlider
//---------------------------------------------------------

VolSlider::VolSlider(QWidget* parent)
   : Slider(parent)
      {
      setLog(true);
//      setRange(-60.0f, 10.0f);
      setRange(-60.0f, 20.0f);
      setScaleWidth(7);
      setLineStep(.8f);
      setPageStep(3.0f);
      setDclickValue1(0.0);
      setDclickValue2(0.0);
      }

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void VolSlider::setValue(double val)
      {
      if (_log) {
            if (qFuzzyIsNull(val))
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
//   value
//---------------------------------------------------------

double VolSlider::value() const
      {
      double val = _log ? (_value <= _minValue) ? 0.0f : pow(10.0, _value*0.05f)
         : _value;
      return val;
      }

}
