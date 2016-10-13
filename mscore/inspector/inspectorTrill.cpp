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
            { P_ID::DIAGONAL,       0, 0, l.diagonal,         l.resetDiagonal         },
            { P_ID::LINE_COLOR,     0, 0, l.lineColor,        l.resetLineColor        },
            { P_ID::LINE_WIDTH,     0, 0, l.lineWidth,        l.resetLineWidth        },
            { P_ID::LINE_STYLE,     0, 0, l.lineStyle,        l.resetLineStyle        },
            { P_ID::DASH_LINE_LEN,  0, 0, l.dashLineLength,   l.resetDashLineLength   },
            { P_ID::DASH_GAP_LEN,   0, 0, l.dashGapLength,    l.resetDashGapLength    },
            { P_ID::TRILL_TYPE,     0, 0, t.trillType,        t.resetTrillType        },
            { P_ID::PLACEMENT,      0, 0, t.placement,        t.resetPlacement        },
            { P_ID::ORNAMENT_STYLE, 0, 0, t.ornamentStyle,    t.resetOrnamentStyle    },
            { P_ID::PLAY,           0, 0, t.playArticulation, t.resetPlayArticulation }
            };

      mapSignals(iiList);
      }
}

