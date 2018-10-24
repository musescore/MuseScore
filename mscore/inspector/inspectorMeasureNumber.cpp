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

#include "inspectorMeasureNumber.h"
#include "libmscore/score.h"
#include "icons.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorMeasureNumber
//---------------------------------------------------------

InspectorMeasureNumber::InspectorMeasureNumber(QWidget* parent)
   : InspectorTextBase(parent)
      {
      const std::vector<InspectorItem> iiList = {
            };
      const std::vector<InspectorPanel> ppList = {
            { t.title, t.panel },
            };

      mapSignals(iiList, ppList);
      }

}

