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

#include "inspectorTextLine.h"
#include "musescore.h"
#include "libmscore/textline.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorTextLine
//---------------------------------------------------------

InspectorTextLine::InspectorTextLine(QWidget* parent)
   : InspectorElementBase(parent)
      {
      l.setupUi(addWidget());
      setupLineStyle(l.lineStyle);
      tl.setupUi(addWidget());

      std::vector<InspectorItem> il = {
            { P_ID::DIAGONAL,      0, 0, l.diagonal,       l.resetDiagonal    },
            { P_ID::LINE_VISIBLE,  0, 0, l.lineVisible,    l.resetLineVisible },
            { P_ID::LINE_COLOR,    0, 0, l.lineColor,      l.resetLineColor   },
            { P_ID::LINE_WIDTH,    0, 0, l.lineWidth,      l.resetLineWidth   },
            { P_ID::LINE_STYLE,    0, 0, l.lineStyle,      l.resetLineStyle   },
            { P_ID::DASH_LINE_LEN, 0, 0, l.dashLineLength, l.resetDashLineLength    },
            { P_ID::DASH_GAP_LEN,  0, 0, l.dashGapLength,  l.resetDashGapLength     },
            { P_ID::PLACEMENT,     0, 0, tl.placement,    tl.resetPlacement  },
            };
      mapSignals(il);
      }
}

