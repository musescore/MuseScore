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

#include "inspectorTremoloBar.h"
#include "tremolobarcanvas.h"
#include "musescore.h"
#include "libmscore/tremolobar.h"
#include "libmscore/undo.h"

namespace Ms {

//---------------------------------------------------------
//   Preset tremolo bars
//---------------------------------------------------------

static const QList<PitchValue> DIP
   = { PitchValue(0, 0),    PitchValue(30, -100), PitchValue(60, 0) };
static const QList<PitchValue> DIVE
   = { PitchValue(0, 0),    PitchValue(60, -150) };
static const QList<PitchValue> RELEASE_UP
   = { PitchValue(0, -150), PitchValue(60, 0)    };
static const QList<PitchValue> INVERTED_DIP
   = { PitchValue(0, 0),    PitchValue(30, 100),  PitchValue(60, 0) };
static const QList<PitchValue> RETURN
   = { PitchValue(0, 0),    PitchValue(60, 150)  };
static const QList<PitchValue> RELEASE_DOWN
   = { PitchValue(0, 150),  PitchValue(60, 0)    };

//---------------------------------------------------------
//   InspectorTremoloBar
//---------------------------------------------------------

InspectorTremoloBar::InspectorTremoloBar(QWidget* parent)
   : InspectorElementBase(parent)
      {
      g.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::PLAY,       0, g.play,        g.resetPlay        },
            { Pid::LINE_WIDTH, 0, g.lineWidth,   g.resetLineWidth   },
            { Pid::MAG,        0, g.mag,         g.resetMag         }
            };
      const std::vector<InspectorPanel> ppList = { { g.title, g.panel } };

      mapSignals(iiList, ppList);

      TremoloBar* tb = toTremoloBar(inspector->element());
      g.tremoloBarCanvas->setPoints(tb->points());
      connect(g.bendType,         SIGNAL(currentIndexChanged(int)),  SLOT(bendTypeChanged(int)));
      connect(g.tremoloBarCanvas, SIGNAL(tremoloBarCanvasChanged()), SLOT(updateBend())        );
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorTremoloBar::setElement()
      {
      InspectorElementBase::setElement();

      QList<PitchValue> points = g.tremoloBarCanvas->points();
      if (!(g.bendType->currentIndex() == 6)) { // custom bend
            if (points == DIP)
                  g.bendType->setCurrentIndex(0);
            else if (points == DIVE)
                  g.bendType->setCurrentIndex(1);
            else if (points == RELEASE_UP)
                  g.bendType->setCurrentIndex(2);
            else if (points == INVERTED_DIP)
                  g.bendType->setCurrentIndex(3);
            else if (points == RETURN)
                  g.bendType->setCurrentIndex(4);
            else if (points == RELEASE_DOWN)
                  g.bendType->setCurrentIndex(5);
            else
                  g.bendType->setCurrentIndex(6);
            }
      }

//---------------------------------------------------------
//   points
//---------------------------------------------------------

const QList<PitchValue>& InspectorTremoloBar::points() const
      {
      return g.tremoloBarCanvas->points();
      }

//---------------------------------------------------------
//   bendTypeChanged
//---------------------------------------------------------

void InspectorTremoloBar::bendTypeChanged(int n)
      {
      QList<PitchValue>& points = g.tremoloBarCanvas->points();

      switch (n) {
         case 0:
            points = DIP;
            break;
         case 1:
            points = DIVE;
            break;
         case 2:
            points = RELEASE_UP;
            break;
         case 3:
            points = INVERTED_DIP;
            break;
         case 4:
            points = RETURN;
            break;
         case 5:
            points = RELEASE_DOWN;
            break;
         case 6:
         default:
            break;
            }

      updateBend();
      update();
      }

//---------------------------------------------------------
//   updateBend
//---------------------------------------------------------

void InspectorTremoloBar::updateBend()
      {
      TremoloBar* tb = toTremoloBar(inspector->element());
      Score* sc = tb->score();
      sc->startCmd();
      for (ScoreElement* t : tb->linkList()) {
            sc->undo(new ChangeTremoloBar(toTremoloBar(t), g.tremoloBarCanvas->points()));
            toTremoloBar(t)->triggerLayout();
            }
      sc->endCmd();
      }

} // namespace Ms
