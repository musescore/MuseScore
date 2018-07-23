//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorDynamic.h"

namespace Ms {

extern void populatePlacement(QComboBox*);

//---------------------------------------------------------
//   InspectorDynamic
//---------------------------------------------------------

InspectorDynamic::InspectorDynamic(QWidget* parent)
   : InspectorTextBase(parent)
      {
      d.setupUi(addWidget());

      const std::vector<InspectorItem> il = {
            { Pid::DYNAMIC_RANGE,    0, d.dynRange,     d.resetDynRange     },
            { Pid::VELOCITY,         0, d.velocity,     0                   },
            { Pid::PLACEMENT,        0, d.placement,    d.resetPlacement    }
            };
      const std::vector<InspectorPanel> ppList = {
            { d.title, d.panel }
            };
      populatePlacement(d.placement);
      mapSignals(il, ppList);
      }

} // namespace Ms

