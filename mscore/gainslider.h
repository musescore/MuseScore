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

#ifndef __GAINSLIDER_H__
#define __GAINSLIDER_H__


#include "awl/fastlog.h"

namespace Ms {

class GainSlider : public QSlider
{
      Q_OBJECT
      double _minValue;
      double _maxValue;

      void setMinLogValue(double min);
      void setMaxLogValue(double max);
      void setLogRange(double min, double max) { setMinLogValue(min); setMaxLogValue(max); };

public:
      GainSlider(QWidget* parent);

      double doubleValue() const;
      void setDoubleValue(double);

      void sliderChange(QAbstractSlider::SliderChange change) override;

signals:
      void doubleValueChanged(double);
      };

}
#endif /* __GAINSLIDER_H__ */
