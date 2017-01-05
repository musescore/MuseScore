//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2013 Werner Schweer
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
   : InspectorElementBase(parent)
      {
      f.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { P_ID::FONT_FACE, 0, 0, f.fontFace, f.resetFontFace },
            { P_ID::FONT_BOLD, 0, 0, f.fontBold, f.resetFontBold }
            };
      const std::vector<InspectorPanel> ppList = {
            { f.title, f.panel }
            };

      mapSignals(iiList, ppList);
      }
}

