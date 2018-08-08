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
#include "inspectorVibrato.h"
#include "musescore.h"
#include "libmscore/vibrato.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorVibrato
//---------------------------------------------------------

InspectorVibrato::InspectorVibrato(QWidget* parent)
   : InspectorElementBase(parent)
      {
      v.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::VIBRATO_TYPE,   0, v.vibratoType,      v.resetVibratoType      },
            { Pid::PLACEMENT,      0, v.placement,        v.resetPlacement        },
            { Pid::PLAY,           0, v.playArticulation, v.resetPlayArticulation }
            };
      const std::vector<InspectorPanel> ppList = {
            { v.title, v.panel }
            };

      populatePlacement(v.placement);
      mapSignals(iiList, ppList);
      }

}

