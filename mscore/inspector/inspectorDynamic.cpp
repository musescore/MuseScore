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
            { Pid::DYNAMIC_RANGE,         0, d.dynRange,                d.resetDynRange     },
            { Pid::VELOCITY,              0, d.velocity,                0                   },
            { Pid::SUB_STYLE,             0, d.style,                   d.resetStyle        },
            { Pid::PLACEMENT,             0, d.placement,               d.resetPlacement    },
            { Pid::VELO_CHANGE,           0, d.changeInVelocity,        0                   },
            { Pid::VELO_CHANGE_SPEED,     0, d.velChangeSpeed,          d.resetVelChangeSpeed }
            };
      const std::vector<InspectorPanel> ppList = {
            { d.title, d.panel }
            };
      populatePlacement(d.placement);
      populateStyle(d.style);
      mapSignals(il, ppList);
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorDynamic::valueChanged(int idx, bool b)
      {
      InspectorTextBase::valueChanged(idx, b);

      Pid pid = iList[idx].t;
      // Update min and max for velocity change input
      if (pid == Pid::VELOCITY) {
            int velocity = d.velocity->value();
            d.changeInVelocity->setMinimum(1 - velocity);
            d.changeInVelocity->setMaximum(127 - velocity);
            }
      }
} // namespace Ms
