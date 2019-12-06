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
      connect(g.bendCanvas,  SIGNAL(bendCanvasChanged()),       SLOT(bendCanvasUpdate())  );
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

      points.clear();
      switch (n) {
         case 0:
            points.append(PitchValue(0, 0));
            points.append(PitchValue(15, 100));
            points.append(PitchValue(60, 100));
            break;
         case 1:
            points.append(PitchValue(0, 0));
            points.append(PitchValue(10, 100));
            points.append(PitchValue(20, 100));
            points.append(PitchValue(30, 0));
            points.append(PitchValue(60, 0));
            break;
         case 2:
            points.append(PitchValue(0, 0));
            points.append(PitchValue(10, 100));
            points.append(PitchValue(20, 100));
            points.append(PitchValue(30, 0));
            points.append(PitchValue(40, 0));
            points.append(PitchValue(50, 100));
            points.append(PitchValue(60, 100));
            break;
         case 3:
            points.append(PitchValue(0, 100));
            points.append(PitchValue(60, 100));
            break;
         case 4:
            points.append(PitchValue(0, 100));
            points.append(PitchValue(15, 100));
            points.append(PitchValue(30, 0));
            points.append(PitchValue(60, 0));
            break;
            }

      bendCanvasUpdate();
      update();
      }

//---------------------------------------------------------
//   bendCanvasUpdate
//---------------------------------------------------------

void InspectorBend::bendCanvasUpdate()
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
