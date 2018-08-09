//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "libmscore/notedot.h"
#include "inspectorNoteDot.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorNoteDot
//---------------------------------------------------------

InspectorNoteDot::InspectorNoteDot(QWidget* parent)
   : InspectorElementBase(parent)
      {
      d.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::DOT_POSITION,   1, d.dotPosition,   d.resetDotPosition   },
            };
      const std::vector<InspectorPanel> ppList = {
            { d.title, d.panel },
            };
      mapSignals(iiList, ppList);
      }
}

