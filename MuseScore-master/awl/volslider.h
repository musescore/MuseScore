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

#ifndef __AWLVOLSLIDER_H__
#define __AWLVOLSLIDER_H__

#include "slider.h"

namespace Awl {

//---------------------------------------------------------
//   VolSlider
//!   Volume Slider entry widget
//
//!   This widget implements a slider with a logarithmic
//!   floating point scale used to adjust the volume
//!   in an audio mixer.
//!   Internal value represents gain as a logarythmic
//!   dB value.
//---------------------------------------------------------

class VolSlider : public Slider {
      Q_OBJECT

   public slots:
      virtual void setValue(double v);

   public:
      VolSlider(QWidget* parent = 0);

      virtual double value() const;
      };

}

#endif

