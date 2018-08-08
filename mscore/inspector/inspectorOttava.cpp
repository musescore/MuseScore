//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspector.h"
#include "inspectorOttava.h"
#include "inspectorTextLine.h"
#include "musescore.h"
#include "libmscore/ottava.h"
#include "libmscore/score.h"
#include "icons.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorOttava
//---------------------------------------------------------

InspectorOttava::InspectorOttava(QWidget* parent)
   : InspectorTextLineBase(parent)
      {
      o.setupUi(addWidget());

      const std::vector<InspectorItem> il = {
            { Pid::OTTAVA_TYPE,             0, o.ottavaType,              0                            },
            { Pid::PLACEMENT,               0, o.placement,               o.resetPlacement             },
            { Pid::NUMBERS_ONLY,            0, o.numbersOnly,             o.resetNumbersOnly           }
            };
      const std::vector<InspectorPanel> ppList = {
            { o.title,  o.panel }
            };
      populatePlacement(o.placement);
      mapSignals(il, ppList);
      }

}

