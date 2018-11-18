//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspector.h"
#include "inspectorLetRing.h"
#include "musescore.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorLetRing
//---------------------------------------------------------

InspectorLetRing::InspectorLetRing(QWidget* parent)
   : InspectorTextLineBase(parent)
      {
      lr.setupUi(addWidget());

      const std::vector<InspectorItem> il = {
            { Pid::PLACEMENT,  0, lr.placement,  lr.resetPlacement             },
            };
      const std::vector<InspectorPanel> ppList = {
            { lr.title, lr.panel },
            };

      populatePlacement(lr.placement);
      mapSignals(il, ppList);
      }
}

