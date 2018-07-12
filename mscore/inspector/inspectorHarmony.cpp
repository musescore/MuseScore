//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorHarmony.h"
#include "musescore.h"
#include "libmscore/harmony.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorHarmony
//---------------------------------------------------------

InspectorHarmony::InspectorHarmony(QWidget* parent)
   : InspectorElementBase(parent)
      {
      b.setupUi(addWidget());

//      Element* e = inspector->element();

      const std::vector<InspectorItem> iiList = {
//            { Pid::AUTOSCALE,         0, b.autoscale,       b.resetAutoscale       },
            };
      const std::vector<InspectorPanel> ppList = { { b.title, b.panel } };

      mapSignals(iiList, ppList);
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorHarmony::valueChanged(int idx)
      {
      InspectorBase::valueChanged(idx);
      }

}

