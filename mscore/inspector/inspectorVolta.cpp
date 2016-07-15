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

#include "inspectorVolta.h"
#include "musescore.h"
#include "libmscore/volta.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorVolta
//---------------------------------------------------------

InspectorVolta::InspectorVolta(QWidget* parent)
   : InspectorElementBase(parent)
      {
      l.setupUi(addWidget());
      setupLineStyle(l.lineStyle);
      //tl.setupUi(addWidget());
      v.setupUi(addWidget());

      std::vector<InspectorItem> il = {
            { P_ID::LINE_VISIBLE,  0, 0, l.lineVisible,    l.resetLineVisible },
            { P_ID::DIAGONAL,      0, 0, l.diagonal,       l.resetDiagonal    },
            { P_ID::LINE_COLOR,    0, 0, l.lineColor,      l.resetLineColor   },
            { P_ID::LINE_WIDTH,    0, 0, l.lineWidth,      l.resetLineWidth   },
            { P_ID::LINE_STYLE,    0, 0, l.lineStyle,      l.resetLineStyle   },
            { P_ID::DASH_LINE_LEN, 0, 0, l.dashLineLength, l.resetDashLineLength },
            { P_ID::DASH_GAP_LEN,  0, 0, l.dashGapLength,  l.resetDashGapLength  },
            // tl
            { P_ID::VOLTA_TYPE,    0, 0, v.voltaType,       v.resetVoltaType  },
            { P_ID::BEGIN_TEXT,    0, 0, v.voltaText,       0  },
            { P_ID::VOLTA_ENDING,  0, 0, v.voltaRepeatList, 0  }
            };

      mapSignals(il);
      }
}

