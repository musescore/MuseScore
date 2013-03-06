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

//---------------------------------------------------------
//   InspectorGlissando
//---------------------------------------------------------

InspectorGlissando::InspectorGlissando(QWidget* parent)
   : InspectorBase(parent)
      {
      QWidget* w = new QWidget;
      b.setupUi(w);
      layout->addWidget(w);

      iList = {
            { P_COLOR,           0, false, b.color,    b.resetColor    },
            { P_VISIBLE,         0, false, b.visible,  b.resetVisible  },
            { P_USER_OFF,        0, false, b.offsetX,  b.resetX        },
            { P_USER_OFF,        1, false, b.offsetY,  b.resetY        },
            { P_GLISS_TYPE,      0, false, b.type,     b.resetType     },
            { P_GLISS_TEXT,      0, false, b.text,     b.resetText     },
            { P_GLISS_SHOW_TEXT, 0, false, b.showText, b.resetShowText }
            };

      mapSignals();
      }

