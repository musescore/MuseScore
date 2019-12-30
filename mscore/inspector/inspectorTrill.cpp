//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorTrill.h"
#include "musescore.h"
#include "libmscore/trill.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorTrill
//---------------------------------------------------------

InspectorTrill::InspectorTrill(QWidget* parent)
   : InspectorElementBase(parent)
      {
      t.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::TRILL_TYPE,     0, t.trillType,        t.resetTrillType        },
            { Pid::PLACEMENT,      0, t.placement,        t.resetPlacement        },
            { Pid::ORNAMENT_STYLE, 0, t.ornamentStyle,    t.resetOrnamentStyle    },
            { Pid::PLAY,           0, t.playArticulation, t.resetPlayArticulation }
            };
      const std::vector<InspectorPanel> ppList = {
            { t.title, t.panel }
            };

      mapSignals(iiList, ppList);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorTrill::setElement()
      {
      InspectorElementBase::setElement();
      if (!t.playArticulation->isChecked()) {
            t.labelOrnamentStyle->setEnabled(false);
            t.ornamentStyle->setEnabled(false);
            t.resetOrnamentStyle->setEnabled(false);
            }
      }
}

