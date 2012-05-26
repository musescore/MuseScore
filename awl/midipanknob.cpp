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

#include "midipanknob.h"

namespace Awl {

//---------------------------------------------------------
//   MidiPanKnob
//---------------------------------------------------------

MidiPanKnob::MidiPanKnob(QWidget* parent)
   : Knob(parent)
      {
      setCenter(true);
      setRange(-64.0f, 63.0f);
      setLineStep(1.0f);
      setPageStep(10.0f);
      }

//---------------------------------------------------------
//   valueChange
//---------------------------------------------------------

void MidiPanKnob::valueChange()
      {
      emit valueChanged(_value + 64.0f, _id);
      update();
      }

}
