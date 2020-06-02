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

#include "inspectorInstrchange.h"

namespace Ms {

extern void populatePlacement(QComboBox*);

//---------------------------------------------------------
//   InspectorInstrumentChange
//---------------------------------------------------------

InspectorInstrumentChange::InspectorInstrumentChange(QWidget* parent)
   : InspectorTextBase(parent)
      {
      ic.setupUi(addWidget());

      const std::vector<InspectorItem> il = {
            { Pid::PLACEMENT,        0, ic.placement,    ic.resetPlacement    }
            };
      const std::vector<InspectorPanel> ppList = {
            { ic.title, ic.panel }
            };
      populatePlacement(ic.placement);
      mapSignals(il, ppList);
      }

} // namespace Ms

