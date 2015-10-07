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

#include "inspectorGlissando.h"
#include "musescore.h"
#include "libmscore/glissando.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorGlissando
//---------------------------------------------------------

InspectorGlissando::InspectorGlissando(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      g.setupUi(addWidget());

      iList = {
            { P_ID::COLOR,           0, false, e.color,    e.resetColor    },
            { P_ID::VISIBLE,         0, false, e.visible,  e.resetVisible  },
            { P_ID::USER_OFF,        0, false, e.offsetX,  e.resetX        },
            { P_ID::USER_OFF,        1, false, e.offsetY,  e.resetY        },
            { P_ID::GLISS_TYPE,      0, false, g.type,     g.resetType     },
            { P_ID::GLISS_TEXT,      0, false, g.text,     g.resetText     },
            { P_ID::GLISS_SHOW_TEXT, 0, false, g.showText, g.resetShowText },
            { P_ID::GLISSANDO_STYLE, 0, false, g.glissandoStyle, g.resetGlissandoStyle},
            { P_ID::PLAY,            0, 0,     g.playGlissando, g.resetPlayGlissando}
            };

      mapSignals();
      }
}

