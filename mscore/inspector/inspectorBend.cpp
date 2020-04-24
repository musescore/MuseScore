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
#include "libmscore/bend.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"

namespace Ms {
     
//---------------------------------------------------------
//   BendType
//---------------------------------------------------------

enum class BendType : char {
      BEND, BEND_RELEASE, BEND_RELEASE_BEND, PREBEND, PREBEND_RELEASE, CUSTOM
      };

static const QList<PitchValue> bend
   = { PitchValue(0, 0),   PitchValue(15, 100), PitchValue(60, 100) };
static const QList<PitchValue> bendRelease
   = { PitchValue(0, 0),   PitchValue(10, 100), PitchValue(20, 100), PitchValue(30, 0), PitchValue(60, 0) };
static const QList<PitchValue> bendReleaseBend
   = { PitchValue(0, 0),   PitchValue(10, 100), PitchValue(20, 100), PitchValue(30, 0), PitchValue(40, 0), PitchValue(50, 100), PitchValue(60, 100) };
static const QList<PitchValue> prebend
   = { PitchValue(0, 100), PitchValue(60, 100) };
static const QList<PitchValue> prebendRelease
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

      g.bendType->clear();
      g.bendType->addItem(tr("Bend"),              int(BendType::BEND)              );
      g.bendType->addItem(tr("Bend/Release"),      int(BendType::BEND_RELEASE)      );
      g.bendType->addItem(tr("Bend/Release/Bend"), int(BendType::BEND_RELEASE_BEND) );
      g.bendType->addItem(tr("Prebend"),           int(BendType::PREBEND)           );
      g.bendType->addItem(tr("Prebend/Release"),   int(BendType::PREBEND_RELEASE)   );
      g.bendType->addItem(tr("Custom"),            int(BendType::CUSTOM)            );

      Bend* b = toBend(inspector->element());
      g.bendCanvas->setPoints(b->points());
      connect(g.bendType,    SIGNAL(currentIndexChanged(int)),  SLOT(bendTypeChanged(int)));
      connect(g.bendCanvas,  SIGNAL(canvasChanged()),           SLOT(updateBend())        );
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorBend::setElement()
      {
      InspectorElementBase::setElement();

      QList<PitchValue> points = g.bendCanvas->points();
      if (!(g.bendType->currentIndex() == int(BendType::CUSTOM))) {  // custom bend
            if (points == bend)
                  g.bendType->setCurrentIndex(int(BendType::BEND));
            else if (points == bendRelease)
                  g.bendType->setCurrentIndex(int(BendType::BEND_RELEASE));
            else if (points == bendReleaseBend)
                  g.bendType->setCurrentIndex(int(BendType::BEND_RELEASE_BEND));
            else if (points == prebend)
                  g.bendType->setCurrentIndex(int(BendType::PREBEND));
            else if (points == prebendRelease)
                  g.bendType->setCurrentIndex(int(BendType::PREBEND_RELEASE));
            else
                  g.bendType->setCurrentIndex(int(BendType::CUSTOM));
            }
      }

//---------------------------------------------------------
//   bendTypeChanged
//---------------------------------------------------------

void InspectorBend::bendTypeChanged(int n)
      {
      switch (n) {
         case 0:
            g.bendCanvas->setPoints(bend);
            break;
         case 1:
            g.bendCanvas->setPoints(bendRelease);
            break;
         case 2:
            g.bendCanvas->setPoints(bendReleaseBend);
            break;
         case 3:
            g.bendCanvas->setPoints(prebend);
            break;
         case 4:
            g.bendCanvas->setPoints(prebendRelease);
            break;
         case 5:
         default:
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
      Bend* b = toBend(inspector->element());
      Score* sc = b->score();
      sc->startCmd();
      for (ScoreElement* el : b->linkList()) {
            sc->undo(new ChangeBend(toBend(el), g.bendCanvas->points()));
            toBend(el)->triggerLayout();
            }
      sc->endCmd();
      }

} // namespace Ms
