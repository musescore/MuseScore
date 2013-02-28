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

      iList[0] = InspectorItem(P_COLOR,       b.color,      b.resetColor);
      iList[1] = InspectorItem(P_VISIBLE,     b.visible,    b.resetVisible);
      iList[2] = InspectorItem(P_USER_OFF, 0, b.offsetX,    b.resetX);
      iList[3] = InspectorItem(P_USER_OFF, 1, b.offsetY,    b.resetY);
      iList[4] = InspectorItem(P_JUMP_TO,     b.jumpTo,     b.resetJumpTo);
      iList[5] = InspectorItem(P_PLAY_UNTIL,  b.playUntil,  b.resetPlayUntil);
      iList[6] = InspectorItem(P_CONTINUE_AT, b.continueAt, b.resetContinueAt);

      mapSignals();
      }

