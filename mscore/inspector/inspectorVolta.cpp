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

#include "inspectorVolta.h"
#include "musescore.h"
#include "libmscore/volta.h"
#include "libmscore/score.h"
#include "icons.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorVolta
//---------------------------------------------------------

InspectorVolta::InspectorVolta(QWidget* parent)
   : InspectorTextLineBase(parent)
      {
      v.setupUi(addWidget());

      const std::vector<InspectorItem> il = {
            { Pid::VOLTA_ENDING,            0, v.voltaRepeatList,        0  }
            };
      const std::vector<InspectorPanel> ppList = {
            { v.title, v.panel }
            };
      mapSignals(il, ppList);
      }

}

