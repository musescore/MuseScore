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
            { Pid::FRET_NUT,     0, f.showNut,     f.resetShowNut     },
            };
      const std::vector<InspectorPanel> ppList = {
            { f.title, f.panel }
            };

      dotTypeButtons = {
            f.circleSelect,
            f.crossSelect,
            f.triangleSelect,
            f.squareSelect
            };

      mapSignals(iiList, ppList);

      FretDiagram* diagram = toFretDiagram(inspector->element());
      int fretNumber = diagram->fretOffset() + 1;
      f.fretNumber->setValue(fretNumber);
      f.resetFretNumber->setEnabled(fretNumber != 1);

      connect(f.fretNumber,      SIGNAL(valueChanged(int)), SLOT(fretNumberChanged(int)));
      connect(f.resetFretNumber, SIGNAL(resetClicked()),    SLOT(resetFretNumber()));

      connect(f.circleSelect,   SIGNAL(toggled(bool)), SLOT(circleButtonToggled(bool)));
      connect(f.crossSelect,    SIGNAL(toggled(bool)), SLOT(crossButtonToggled(bool)));
      connect(f.triangleSelect, SIGNAL(toggled(bool)), SLOT(triangleButtonToggled(bool)));
      connect(f.squareSelect,   SIGNAL(toggled(bool)), SLOT(squareButtonToggled(bool)));
      connect(f.toggleBarre,    SIGNAL(toggled(bool)), SLOT(barreButtonToggled(bool)));
      connect(f.toggleMultidot, SIGNAL(toggled(bool)), SLOT(multidotButtonToggled(bool)));
      connect(f.clearButton,    SIGNAL(clicked()),     SLOT(clearButtonClicked()));
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

//---------------------------------------------------------
//   genericButtonToggled
//---------------------------------------------------------

void InspectorFretDiagram::genericButtonToggled(QPushButton* b, bool v, FretDotType dtype)
      {
      for (QPushButton* p : dotTypeButtons) {
            p->blockSignals(true);
            p->setChecked(false);
            }

      if (v) {
            f.diagram->setCurrentDotType(dtype);
            b->setChecked(true);
            }
      
      f.diagram->setAutomaticDotType(!v);

      for (QPushButton* p : dotTypeButtons)
            p->blockSignals(false);
      }
//---------------------------------------------------------
//   circleButtonToggled
//---------------------------------------------------------

void InspectorFretDiagram::circleButtonToggled(bool v) 
      {
      genericButtonToggled(f.circleSelect, v, FretDotType::NORMAL);
      }

//---------------------------------------------------------
//   crossButtonToggled
//---------------------------------------------------------

void InspectorFretDiagram::crossButtonToggled(bool v) 
      {
      genericButtonToggled(f.crossSelect, v, FretDotType::CROSS);
      }

//---------------------------------------------------------
//   squareButtonToggled
//---------------------------------------------------------

void InspectorFretDiagram::squareButtonToggled(bool v) 
      {
      genericButtonToggled(f.squareSelect, v, FretDotType::SQUARE);
      }

//---------------------------------------------------------
//   triangleButtonToggled
//---------------------------------------------------------

void InspectorFretDiagram::triangleButtonToggled(bool v) 
      {
      genericButtonToggled(f.triangleSelect, v, FretDotType::TRIANGLE);
      }

//---------------------------------------------------------
//   barreButtonToggled
//---------------------------------------------------------

void InspectorFretDiagram::barreButtonToggled(bool v) 
      {
      f.diagram->setBarreMode(v);
      }

//---------------------------------------------------------
//   multidotButtonToggled
//---------------------------------------------------------

void InspectorFretDiagram::multidotButtonToggled(bool v) 
      {
      f.diagram->setMultidotMode(v);
      }

//---------------------------------------------------------
//   clearButtonClicked
//---------------------------------------------------------

void InspectorFretDiagram::clearButtonClicked() 
      {
      f.diagram->clear();
      }
}

