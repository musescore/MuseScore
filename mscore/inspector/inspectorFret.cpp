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
            { Pid::COLOR,        0, e.color,       e.resetColor       },
            { Pid::VISIBLE,      0, e.visible,     e.resetVisible     },
            { Pid::OFFSET,       0, e.offset,      e.resetOffset      },
            { Pid::MAG,          0, f.mag,         f.resetMag         },
            { Pid::PLACEMENT,    0, f.placement,   f.resetPlacement   },
            { Pid::FRET_STRINGS, 0, f.strings,     f.resetStrings     },
            { Pid::FRET_FRETS,   0, f.frets,       f.resetFrets       },
            { Pid::FRET_OFFSET,  0, f.offset,      f.resetOffset      },
            { Pid::FRET_BARRE,   0, f.barre,       f.resetBarre       },
            };
      const std::vector<InspectorPanel> ppList = {
            { f.title, f.panel }
            };
      mapSignals(iiList, ppList);
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorFretDiagram::valueChanged(int idx)
      {
      InspectorElementBase::valueChanged(idx);
      FretDiagram* fd = toFretDiagram(inspector->element());
      f.diagram->setFretDiagram(fd);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorFretDiagram::setElement()
      {
      InspectorElementBase::setElement();
      FretDiagram* fd = toFretDiagram(inspector->element());
      f.diagram->setFretDiagram(fd);
      }

}

