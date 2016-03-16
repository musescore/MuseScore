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

#include "inspectorHairpin.h"
#include "musescore.h"
#include "libmscore/hairpin.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorHairpin
//---------------------------------------------------------

InspectorHairpin::InspectorHairpin(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      l.setupUi(addWidget());
      setupLineStyle(l.lineStyle);
      h.setupUi(addWidget());

      iList = {
            { P_ID::COLOR,               0, 0, e.color,             e.resetColor             },
            { P_ID::VISIBLE,             0, 0, e.visible,           e.resetVisible           },
            { P_ID::USER_OFF,            0, 0, e.offsetX,           e.resetX                 },
            { P_ID::USER_OFF,            1, 0, e.offsetY,           e.resetY                 },
            { P_ID::DIAGONAL,            0, 0, l.diagonal,          l.resetDiagonal          },
            { P_ID::LINE_VISIBLE,        0, 0, l.lineVisible,       l.resetLineVisible       },
            { P_ID::LINE_COLOR,          0, 0, l.lineColor,         l.resetLineColor         },
            { P_ID::LINE_WIDTH,          0, 0, l.lineWidth,         l.resetLineWidth         },
            { P_ID::LINE_STYLE,          0, 0, l.lineStyle,         l.resetLineStyle         },
            { P_ID::HAIRPIN_TEXTLINE,    0, 0, h.useTextLine,       h.resetUseTextLine       },
            { P_ID::HAIRPIN_CIRCLEDTIP,  0, 0, h.hairpinCircledTip, h.resetHairpinCircledTip },
            { P_ID::HAIRPIN_TYPE,        0, 0, h.hairpinType,       h.resetHairpinType       },
            { P_ID::DYNAMIC_RANGE,       0, 0, h.dynRange,          h.resetDynRange          },
            { P_ID::VELO_CHANGE,         0, 0, h.veloChange,        h.resetVeloChange        },
            { P_ID::HAIRPIN_HEIGHT,      0, 0, h.hairpinHeight,     h.resetHairpinHeight     },
            { P_ID::HAIRPIN_CONT_HEIGHT, 0, 0, h.hairpinContHeight, h.resetHairpinContHeight }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   postInit
//---------------------------------------------------------

void InspectorHairpin::postInit()
      {
      bool useTextLine = h.useTextLine->isChecked();
      l.lineVisible->setEnabled(useTextLine);
      h.hairpinCircledTip->setDisabled(useTextLine);
      h.hairpinHeight->setDisabled(useTextLine);
      h.hairpinContHeight->setDisabled(useTextLine);
      }

}

