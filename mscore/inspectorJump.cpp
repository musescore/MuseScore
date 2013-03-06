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

#include "inspectorJump.h"
#include "musescore.h"
#include "libmscore/jump.h"
#include "libmscore/score.h"

//---------------------------------------------------------
//   InspectorJump
//---------------------------------------------------------

InspectorJump::InspectorJump(QWidget* parent)
   : InspectorBase(parent)
      {
      QWidget* w = new QWidget;
      b.setupUi(w);
      layout->addWidget(w);

      iList = {
            { P_COLOR,       0, false, b.color,      b.resetColor      },
            { P_VISIBLE,     0, false, b.visible,    b.resetVisible    },
            { P_USER_OFF,    0, false, b.offsetX,    b.resetX          },
            { P_USER_OFF,    1, false, b.offsetY,    b.resetY          },
            { P_JUMP_TO,     0, false, b.jumpTo,     b.resetJumpTo     },
            { P_PLAY_UNTIL,  0, false, b.playUntil,  b.resetPlayUntil  },
            { P_CONTINUE_AT, 0, false, b.continueAt, b.resetContinueAt }
            };

      mapSignals();
      }

