//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorArpeggio.h"
#include "musescore.h"
#include "libmscore/arpeggio.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorArpeggio
//---------------------------------------------------------

InspectorArpeggio::InspectorArpeggio(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      g.setupUi(addWidget());

      iList = {
            { P_ID::COLOR,           0, false, e.color,    e.resetColor    },
            { P_ID::VISIBLE,         0, false, e.visible,  e.resetVisible  },
            { P_ID::USER_OFF,        0, false, e.offsetX,  e.resetX        },
            { P_ID::USER_OFF,        1, false, e.offsetY,  e.resetY        },

            { P_ID::PLAY,            0, 0,     g.playArpeggio, g.resetPlayArpeggio}
            };

      mapSignals();
      }
}

