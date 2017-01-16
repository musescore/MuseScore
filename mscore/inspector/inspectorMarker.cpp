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

#include "inspectorMarker.h"
#include "musescore.h"
#include "libmscore/marker.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   inspectorMarker
//---------------------------------------------------------

InspectorMarker::InspectorMarker(QWidget* parent)
   : InspectorElementBase(parent)
      {
      t.setupUi(addWidget());
      m.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { P_ID::MARKER_TYPE,        0, false, m.markerType, 0            },
            { P_ID::LABEL,              0, false, m.jumpLabel,  0            }
            };
      const std::vector<InspectorPanel> ppList = {
            { t.title, t.panel },
            { m.title, m.panel }
            };
      mapSignals(iiList, ppList);
      connect(t.resetToStyle, SIGNAL(clicked()), SLOT(resetToStyle()));
      }

}

