//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorFret.h"
#include "musescore.h"
#include "libmscore/fret.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorFretDiagram
//---------------------------------------------------------

InspectorFretDiagram::InspectorFretDiagram(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      f.setupUi(addWidget());

      iList = {
            { P_ID::COLOR,        0, 0, e.color,       e.resetColor      },
            { P_ID::VISIBLE,      0, 0, e.visible,     e.resetVisible    },
            { P_ID::USER_OFF,     0, 0, e.offsetX,     e.resetX          },
            { P_ID::USER_OFF,     1, 0, e.offsetY,     e.resetY          },
            { P_ID::MAG,          0, 0, f.mag,         f.resetMag        },
            { P_ID::FRET_STRINGS, 0, 0, f.strings,     f.resetStrings    },
            { P_ID::FRET_FRETS,   0, 0, f.frets,       f.resetFrets      },
            { P_ID::FRET_BARRE,   0, 0, f.barre,       f.resetBarre      }
            };

      mapSignals();
      }
}

