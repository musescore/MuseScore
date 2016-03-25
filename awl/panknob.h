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

#ifndef __AWLPANKNOB_H__
#define __AWLPANKNOB_H__

#include "knob.h"

namespace Awl {

//---------------------------------------------------------
//   PanKnob
//!   Pan Knob entry widget
//
//!   This widget implements a centered floating point
//!   knob used to adjust the pan position in an audio
//!   mixer.
//---------------------------------------------------------

class PanKnob : public Knob {
      Q_OBJECT

   public:
      PanKnob(QWidget* parent = 0);
      };
}

#endif

