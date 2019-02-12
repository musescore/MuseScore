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

#include "inspectorJump.h"
#include "musescore.h"
#include "libmscore/jump.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorJump
//---------------------------------------------------------

InspectorJump::InspectorJump(QWidget* parent)
   : InspectorTextBase(parent)
      {
      j.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::JUMP_TO,            0, j.jumpTo,      0                  },
            { Pid::PLAY_UNTIL,         0, j.playUntil,   0                  },
            { Pid::CONTINUE_AT,        0, j.continueAt,  0                  },
            { Pid::PLAY_REPEATS,       0, j.playRepeats, j.resetPlayRepeats }
            };
      const std::vector<InspectorPanel> ppList = {
            { t.title, t.panel },
            { j.title, j.panel }
            };

      mapSignals(iiList, ppList);
      }

}
