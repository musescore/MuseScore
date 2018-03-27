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

namespace Ms {

//---------------------------------------------------------
//   InspectorTrill
//---------------------------------------------------------

InspectorTrill::InspectorTrill(QWidget* parent)
   : InspectorElementBase(parent)
      {
      l.setupUi(addWidget());
      setupLineStyle(l.lineStyle);
      t.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::DIAGONAL,       0, l.diagonal,         l.resetDiagonal         },
            { Pid::LINE_COLOR,     0, l.lineColor,        l.resetLineColor        },
            { Pid::LINE_WIDTH,     0, l.lineWidth,        l.resetLineWidth        },
            { Pid::LINE_STYLE,     0, l.lineStyle,        l.resetLineStyle        },
            { Pid::DASH_LINE_LEN,  0, l.dashLineLength,   l.resetDashLineLength   },
            { Pid::DASH_GAP_LEN,   0, l.dashGapLength,    l.resetDashGapLength    },
            { Pid::TRILL_TYPE,     0, t.trillType,        t.resetTrillType        },
            { Pid::PLACEMENT,      0, t.placement,        t.resetPlacement        },
            { Pid::ORNAMENT_STYLE, 0, t.ornamentStyle,    t.resetOrnamentStyle    },
            { Pid::PLAY,           0, t.playArticulation, t.resetPlayArticulation }
            };
      const std::vector<InspectorPanel> ppList = {
            { l.title, l.panel },
            { t.title, t.panel }
            };

      mapSignals(iiList, ppList);
      }
}

