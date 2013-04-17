//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorMarker.h"
#include "musescore.h"
#include "libmscore/marker.h"
#include "libmscore/score.h"

//---------------------------------------------------------
//   inspectorMarker
//---------------------------------------------------------

InspectorMarker::InspectorMarker(QWidget* parent)
   : InspectorBase(parent)
      {
      b.setupUi(addWidget());
      m.setupUi(addWidget());

      iList = {
            { P_COLOR,       0, false, b.color,      b.resetColor      },
            { P_VISIBLE,     0, false, b.visible,    b.resetVisible    },
            { P_USER_OFF,    0, false, b.offsetX,    b.resetX          },
            { P_USER_OFF,    1, false, b.offsetY,    b.resetY          },
            { P_MARKER_TYPE, 0, false, m.markerType, m.resetMarkerType },
            { P_LABEL,       0, false, m.jumpLabel,  m.resetJumpLabel  },
            };

      mapSignals();
      }

