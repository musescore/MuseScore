//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorArpeggio.h"
#include "musescore.h"
#include "libmscore/arpeggio.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorArpeggio
//---------------------------------------------------------

InspectorArpeggio::InspectorArpeggio(QWidget* parent)
   : InspectorElementBase(parent)
      {
      g.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::PLAY,            0,    g.playArpeggio, g.resetPlayArpeggio},
            { Pid::TIME_STRETCH,    0,    g.stretch,      g.resetStretch }
            };
      const std::vector<InspectorPanel> ppList = {
            { g.title, g.panel }
            };

      mapSignals(iiList, ppList);
      }
}

