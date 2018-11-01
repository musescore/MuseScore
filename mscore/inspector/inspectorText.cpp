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

#include "inspectorText.h"
#include "libmscore/score.h"
#include "icons.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorText
//---------------------------------------------------------

InspectorText::InspectorText(QWidget* parent)
   : InspectorTextBase(parent)
      {
      f.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
//            { Pid::SUB_STYLE, 0, f.style,     f.resetStyle     },
            { Pid::SUB_STYLE, 0, f.style,     0     },
            };

      const std::vector<InspectorPanel> ppList = {
            { f.title, f.panel }
            };

      populateStyle(f.style);
      mapSignals(iiList, ppList);
      }

}

