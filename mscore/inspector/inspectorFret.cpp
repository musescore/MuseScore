//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorFret.h"
#include "musescore.h"
#include "libmscore/fret.h"
#include "libmscore/score.h"
#include "fretproperties.h"
#include "scoreview.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorFretDiagram
//---------------------------------------------------------

InspectorFretDiagram::InspectorFretDiagram(QWidget* parent)
   : InspectorElementBase(parent)
      {
      f.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::COLOR,    0, e.color,       e.resetColor      },
            { Pid::VISIBLE,  0, e.visible,     e.resetVisible    },
            { Pid::USER_OFF, 0, e.offset,      e.resetOffset     },
            { Pid::MAG,      0, f.mag,         f.resetMag        }
            };
      const std::vector<InspectorPanel> ppList = {
            { f.title, f.panel }
            };
      mapSignals(iiList, ppList);
      connect(f.properties, SIGNAL(clicked()), SLOT(propertiesClicked()));
      }

//---------------------------------------------------------
//   propertiesClicked
//---------------------------------------------------------

void InspectorFretDiagram::propertiesClicked()
      {
      FretDiagram* fd = static_cast<FretDiagram*>(inspector->element());
      Score* score = fd->score();
      score->startCmd();
      mscore->currentScoreView()->editFretDiagram(fd);
      score->setLayoutAll();
      score->endCmd();
      }

}

