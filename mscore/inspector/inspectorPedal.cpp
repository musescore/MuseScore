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
#include "inspectorPedal.h"
#include "musescore.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorPedal
//---------------------------------------------------------

InspectorPedal::InspectorPedal(QWidget* parent)
   : InspectorTextLineBase(parent)
      {
      p.setupUi(addWidget());

      const std::vector<InspectorItem> il = {
            { Pid::PLACEMENT,  0, p.placement,  p.resetPlacement             },
            };
      const std::vector<InspectorPanel> ppList = {
            { p.title, p.panel },
            };

      populatePlacement(p.placement);
      mapSignals(il, ppList);
      }
}

