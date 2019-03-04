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
            { Pid::FRET_BARRE,   0, f.barre,       f.resetBarre       },
            };
      const std::vector<InspectorPanel> ppList = {
            { f.title, f.panel }
            };
      mapSignals(iiList, ppList);
      int fretNumber = toFretDiagram(inspector->element())->fretOffset() + 1;
      f.fretNumber->setValue(fretNumber);
      connect(f.fretNumber, SIGNAL(valueChanged(int)), SLOT(fretNumberChanged(int)));
      connect(f.resetFretNumber, SIGNAL(resetClicked()), SLOT(resetFretNumber()));
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

//---------------------------------------------------------
//   fretNumberChanged
//---------------------------------------------------------

void InspectorFretDiagram::fretNumberChanged(int fretNumber)
      {
      FretDiagram* fd = toFretDiagram(inspector->element());
      fd->score()->startCmd();
      fd->undoChangeProperty(Pid::FRET_OFFSET, fretNumber - 1);
      fd->score()->endCmd();
      f.resetFretNumber->setEnabled(fretNumber != 1);
      f.diagram->setFretDiagram(fd);
      }

//---------------------------------------------------------
//   resetFretNumberClicked
//---------------------------------------------------------

void InspectorFretDiagram::resetFretNumber()
      {
      FretDiagram* fd = toFretDiagram(inspector->element());
      int fretNumber = 1;
      fd->score()->startCmd();
      fd->undoChangeProperty(Pid::FRET_OFFSET, fretNumber - 1);
      fd->score()->endCmd();
      f.fretNumber->setValue(fretNumber);
      f.resetFretNumber->setEnabled(false);
      f.diagram->setFretDiagram(fd);
      }

}

