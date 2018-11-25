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
#include "musescore.h"
#include "libmscore/harmony.h"
#include "libmscore/score.h"
#include "icons.h"

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
            { Pid::PLACEMENT, 0, h.placement,   h.resetPlacement  }
            };

      const std::vector<InspectorPanel> ppList = {
            { h.title, h.panel }
            };

      h.style->clear();
      for (auto ss : primaryTextStyles())
            h.style->addItem(textStyleUserName(ss), int(ss));

      t.resetToStyle->setVisible(false);

      mapSignals(iiList, ppList);
      }

}

