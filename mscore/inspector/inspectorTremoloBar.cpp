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
//   TremoloBarType
//---------------------------------------------------------

enum class TremoloBarType : char {
      DIP, DIVE, RELEASE_UP, INVERTED_DIP, RETURN, RELEASE_DOWN, CUSTOM
      };

static const QList<PitchValue> Dip
   = { PitchValue(0, 0),    PitchValue(30, -100), PitchValue(60, 0) };
static const QList<PitchValue> Dive
   = { PitchValue(0, 0),    PitchValue(60, -150) };
static const QList<PitchValue> ReleaseUp
   = { PitchValue(0, -150), PitchValue(60, 0)    };
static const QList<PitchValue> InvertedDip
   = { PitchValue(0, 0),    PitchValue(30, 100),  PitchValue(60, 0) };
static const QList<PitchValue> Return
   = { PitchValue(0, 0),    PitchValue(60, 150)  };
static const QList<PitchValue> ReleaseDown
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

      g.tremoloBarType->clear();
      g.tremoloBarType->addItem(tr("Dip"),            int(TremoloBarType::DIP)          );
      g.tremoloBarType->addItem(tr("Dive"),           int(TremoloBarType::DIVE)         );
      g.tremoloBarType->addItem(tr("Release (Up)"),   int(TremoloBarType::RELEASE_UP)   );
      g.tremoloBarType->addItem(tr("Inverted Dip"),   int(TremoloBarType::INVERTED_DIP) );
      g.tremoloBarType->addItem(tr("Return"),         int(TremoloBarType::RETURN)       );
      g.tremoloBarType->addItem(tr("Release (Down)"), int(TremoloBarType::RELEASE_DOWN) );
      g.tremoloBarType->addItem(tr("Custom"),         int(TremoloBarType::CUSTOM)       );

      TremoloBar* tb = toTremoloBar(inspector->element());
      g.tremoloBarCanvas->setPoints(tb->points());
      connect(g.tremoloBarType,   SIGNAL(currentIndexChanged(int)),  SLOT(tremoloBarTypeChanged(int)));
      connect(g.tremoloBarCanvas, SIGNAL(canvasChanged()), SLOT(updateTremoloBar())        );
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorTremoloBar::setElement()
      {
      InspectorElementBase::setElement();

      QList<PitchValue> points = g.tremoloBarCanvas->points();
      if (!(g.tremoloBarType->currentIndex() == int(TremoloBarType::CUSTOM))) { // custom tremolo bar
            if (points == Dip)
                  g.tremoloBarType->setCurrentIndex(int(TremoloBarType::DIP));
            else if (points == Dive)
                  g.tremoloBarType->setCurrentIndex(int(TremoloBarType::DIVE));
            else if (points == ReleaseUp)
                  g.tremoloBarType->setCurrentIndex(int(TremoloBarType::RELEASE_UP));
            else if (points == InvertedDip)
                  g.tremoloBarType->setCurrentIndex(int(TremoloBarType::INVERTED_DIP));
            else if (points == Return)
                  g.tremoloBarType->setCurrentIndex(int(TremoloBarType::RETURN));
            else if (points == ReleaseDown)
                  g.tremoloBarType->setCurrentIndex(int(TremoloBarType::RELEASE_DOWN));
            else
                  g.tremoloBarType->setCurrentIndex(int(TremoloBarType::CUSTOM));
            }
      }

//---------------------------------------------------------
//   tremoloBarTypeChange
//---------------------------------------------------------

void InspectorTremoloBar::tremoloBarTypeChanged(int n)
      {
      switch (n) {
         case 0:
            g.tremoloBarCanvas->setPoints(Dip);
            break;
         case 1:
            g.tremoloBarCanvas->setPoints(Dive);
            break;
         case 2:
            g.tremoloBarCanvas->setPoints(ReleaseUp);
            break;
         case 3:
            g.tremoloBarCanvas->setPoints(InvertedDip);
            break;
         case 4:
            g.tremoloBarCanvas->setPoints(Return);
            break;
         case 5:
            g.tremoloBarCanvas->setPoints(ReleaseDown);
            break;
         case 6:
         default:
            break;
            }

      updateTremoloBar();
      update();
      }

//---------------------------------------------------------
//   updateTremoloBar
//---------------------------------------------------------

void InspectorTremoloBar::updateTremoloBar()
      {
      TremoloBar* tb = toTremoloBar(inspector->element());
      Score* sc = tb->score();
      sc->startCmd();
      for (ScoreElement* el : tb->linkList()) {
            sc->undo(new ChangeTremoloBar(toTremoloBar(el), g.tremoloBarCanvas->points()));
            toTremoloBar(el)->triggerLayout();
            }
      sc->endCmd();
      }

} // namespace Ms
