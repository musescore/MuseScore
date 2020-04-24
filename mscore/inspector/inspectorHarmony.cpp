//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorHarmony.h"
#include "libmscore/harmony.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorHarmony
//---------------------------------------------------------

InspectorHarmony::InspectorHarmony(QWidget* parent)
   : InspectorTextBase(parent)
      {
      h.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::SUB_STYLE, 0, h.style,       h.resetStyle      },
            { Pid::PLACEMENT, 0, h.placement,   h.resetPlacement  },
            { Pid::PLAY,      0, h.play,        h.resetPlay  },
            { Pid::HARMONY_VOICE_LITERAL, 0, h.voicingSelect->interpretBox, h.resetVoicing },
            { Pid::HARMONY_VOICING, 0, h.voicingSelect->voicingBox, h.resetVoicing},
            { Pid::HARMONY_DURATION, 0, h.voicingSelect->durationBox, h.resetVoicing}
            };

      const std::vector<InspectorPanel> ppList = {
            { h.title, h.panel }
            };

      populateStyle(h.style);
      t.resetToStyle->setVisible(false);
      mapSignals(iiList, ppList);
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorHarmony::valueChanged(int idx, bool b)
      {
      InspectorTextBase::valueChanged(idx, b);
      if (iList[idx].t == Pid::PLAY) {
            bool playChecked = h.play->isChecked();
            h.voicingSelect->setVisible(playChecked);
            h.resetVoicing->setVisible(playChecked);
            }
      }
}

