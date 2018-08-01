//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorFingering.h"
#include "musescore.h"
#include "libmscore/fingering.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorFingering
//---------------------------------------------------------

InspectorFingering::InspectorFingering(QWidget* parent)
   : InspectorTextBase(parent)
      {
      f.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::SUB_STYLE, 0, f.style,     f.resetStyle     },
            };
      const std::vector<InspectorPanel> ppList = {
            { f.title, f.panel }
            };

      f.style->clear();
      for (auto ss : { Tid::FINGERING, Tid::LH_GUITAR_FINGERING, Tid::RH_GUITAR_FINGERING, Tid::STRING_NUMBER } )
            {
            f.style->addItem(textStyleUserName(ss), int(ss));
            }

      mapSignals(iiList, ppList);
      }
}

