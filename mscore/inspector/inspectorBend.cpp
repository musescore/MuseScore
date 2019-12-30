//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorBend.h"
#include "bendcanvas.h"
#include "musescore.h"
#include "libmscore/bend.h"
#include "libmscore/undo.h"

namespace Ms {
     
//---------------------------------------------------------
//   Preset bends
//---------------------------------------------------------

static const QList<PitchValue> BEND 
   = { PitchValue(0, 0),   PitchValue(15, 100), PitchValue(60, 100) };
static const QList<PitchValue> BEND_RELEASE
   = { PitchValue(0, 0),   PitchValue(10, 100), PitchValue(20, 100), PitchValue(30, 0), PitchValue(60, 0) };
static const QList<PitchValue> BEND_RELEASE_BEND
   = { PitchValue(0, 0),   PitchValue(10, 100), PitchValue(20, 100), PitchValue(30, 0), PitchValue(40, 0), PitchValue(50, 100), PitchValue(60, 100) };
static const QList<PitchValue> PREBEND
   = { PitchValue(0, 100), PitchValue(60, 100) };
static const QList<PitchValue> PREBEND_RELEASE
   = { PitchValue(0, 100), PitchValue(15, 100), PitchValue(30, 0),   PitchValue(60, 0) };

//---------------------------------------------------------
//   InspectorBend
//---------------------------------------------------------

InspectorBend::InspectorBend(QWidget* parent)
   : InspectorElementBase(parent)
      {
      g.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::LINE_WIDTH,     0, g.lineWidth,   g.resetLineWidth   },
            { Pid::PLAY,           0, g.playBend,    g.resetPlayBend    },
            { Pid::FONT_FACE,      0, g.fontFace,    g.resetFontFace    },
            { Pid::FONT_SIZE,      0, g.fontSize,    g.resetFontSize    },
            { Pid::FONT_STYLE,     0, g.fontStyle,   g.resetFontStyle   },
            };
      const std::vector<InspectorPanel> ppList = { {g.title, g.panel} };
      mapSignals(iiList, ppList);

      Bend* b = toBend(inspector->element());
      g.bendCanvas->setPoints(b->points());
      connect(g.bendType,    SIGNAL(currentIndexChanged(int)),  SLOT(bendTypeChanged(int)));
      connect(g.bendCanvas,  SIGNAL(bendCanvasChanged()),       SLOT(updateBend())        );
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorBend::setElement()
      {
      InspectorElementBase::setElement();

      QList<PitchValue> points = g.bendCanvas->points();
      if (!(g.bendType->currentIndex() == 5)) {
            if (points == BEND)
                  g.bendType->setCurrentIndex(0);
            else if (points == BEND_RELEASE)
                  g.bendType->setCurrentIndex(1);
            else if (points == BEND_RELEASE_BEND)
                  g.bendType->setCurrentIndex(2);
            else if (points == PREBEND)
                  g.bendType->setCurrentIndex(3);
            else if (points == PREBEND_RELEASE)
                  g.bendType->setCurrentIndex(4);
            else
                  g.bendType->setCurrentIndex(5);
            }
      }

//---------------------------------------------------------
//   points
//---------------------------------------------------------

const QList<PitchValue>& InspectorBend::points() const
      {
      return g.bendCanvas->points();
      }

//---------------------------------------------------------
//   bendTypeChanged
//---------------------------------------------------------

void InspectorBend::bendTypeChanged(int n)
      {
      QList<PitchValue>& points = g.bendCanvas->points();

      switch (n) {
         case 0:
            points = BEND;
            break;
         case 1:
            points = BEND_RELEASE;
            break;
         case 2:
            points = BEND_RELEASE_BEND;
            break;
         case 3:
            points = PREBEND;
            break;
         case 4:
            points = PREBEND_RELEASE;
            break;
         case 5:
            break;
            }

      updateBend();
      update();
      }

//---------------------------------------------------------
//   updateBend
//---------------------------------------------------------

void InspectorBend::updateBend()
      {
      Bend* bend = toBend(inspector->element());
      Score* sc = bend->score();
      sc->startCmd();
      for (ScoreElement* b : bend->linkList()) {
            sc->undo(new ChangeBend(toBend(b), g.bendCanvas->points()));
            toBend(b)->triggerLayout();
            }
      sc->endCmd();
      }

} // namespace Ms
