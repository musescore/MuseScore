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

#ifndef __AWLMIDIPANKNOB_H__
#define __AWLMIDIPANKNOB_H__

#include "knob.h"

namespace Awl {

//---------------------------------------------------------
//   MidiPanKnob
//!   MidiPan Knob entry widget
//
//!   This widget implements a centered floating point
//!   knob used to adjust the pan position in an audio
//!   mixer.
//---------------------------------------------------------

class MidiPanKnob : public Knob {
      Q_OBJECT

      virtual void valueChange();

   public slots:
      virtual void setValue(double v) { AbstractSlider::setValue(v - 64.0f); }

   public:
      MidiPanKnob(QWidget* parent = 0);
      virtual double value() const { return _value + 64.0f; }
      };
}

#endif

