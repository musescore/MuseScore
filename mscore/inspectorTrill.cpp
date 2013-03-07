//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorTrill.h"
#include "musescore.h"
#include "libmscore/trill.h"
#include "libmscore/score.h"

//---------------------------------------------------------
//   InspectorTrill
//---------------------------------------------------------

InspectorTrill::InspectorTrill(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      t.setupUi(addWidget());

      iList = {
            { P_COLOR,       0, 0, e.color,      e.resetColor      },
            { P_VISIBLE,     0, 0, e.visible,    e.resetVisible    },
            { P_USER_OFF,    0, 0, e.offsetX,    e.resetX          },
            { P_USER_OFF,    1, 0, e.offsetY,    e.resetY          },
            { P_TRILL_TYPE,  0, 0, t.trillType,  t.resetTrillType  }
            };

      mapSignals();
      }

