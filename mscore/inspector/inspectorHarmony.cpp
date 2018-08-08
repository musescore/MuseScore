//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
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
            { Pid::POS_ABOVE, 0, h.posAbove, h.resetPosAbove },
            { Pid::SUB_STYLE, 0, h.style, h.resetStyle       },
            };

      const std::vector<InspectorPanel> ppList = {
            { h.title, h.panel }
            };

      h.style->clear();
      for (auto ss : { Tid::HARMONY_A, Tid::HARMONY_B } ) {
            h.style->addItem(textStyleUserName(ss), int(ss));
            }

      t.resetToStyle->setVisible(false);

      mapSignals(iiList, ppList);
      }

}

