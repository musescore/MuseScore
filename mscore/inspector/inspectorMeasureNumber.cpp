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
#include "libmscore/measurenumber.h"

namespace Ms {

extern void populatePlacement(QComboBox*);

//---------------------------------------------------------
//   InspectorMeasureNumber
//---------------------------------------------------------

InspectorMeasureNumber::InspectorMeasureNumber(QWidget* parent)
   : InspectorTextBase(parent)
      {
      mn.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::SUB_STYLE,  0, mn.style,      mn.resetStyle },
            { Pid::PLACEMENT,  0, mn.vPlacement, mn.resetVPlacement  },
            { Pid::HPLACEMENT, 0, mn.hPlacement, mn.resetHPlacement  }
            };
      const std::vector<InspectorPanel> ppList = {
            { mn.title, mn.panel },
            };

      populateStyle(mn.style);
      populatePlacement(mn.vPlacement);

      mn.hPlacement->clear();
      mn.hPlacement->addItem(mn.hPlacement->QObject::tr("Left"),   int(HPlacement::LEFT));
      mn.hPlacement->addItem(mn.hPlacement->QObject::tr("Center"), int(HPlacement::CENTER));
      mn.hPlacement->addItem(mn.hPlacement->QObject::tr("Right"),  int(HPlacement::RIGHT));

      mapSignals(iiList, ppList);
      }

} // namespace Ms

