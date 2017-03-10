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
            { P_ID::SUB_STYLE, 0, f.subStyle,     f.resetSubStyle     },
            };
      const std::vector<InspectorPanel> ppList = {
            { f.title, f.panel }
            };

      f.subStyle->clear();
      for (auto ss : { SubStyle::FINGERING, SubStyle::LH_GUITAR_FINGERING, SubStyle::RH_GUITAR_FINGERING, SubStyle::STRING_NUMBER } )
            {
            f.subStyle->addItem(subStyleUserName(ss), int(ss));
            }

      mapSignals(iiList, ppList);
      }
}

