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

#include "inspectorGlissando.h"
#include "musescore.h"
#include "libmscore/glissando.h"
#include "libmscore/score.h"
#include "icons.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorGlissando
//---------------------------------------------------------

InspectorGlissando::InspectorGlissando(QWidget* parent)
   : InspectorElementBase(parent)
      {
      g.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::GLISS_TYPE,      0, g.type,           g.resetType           },
            { Pid::GLISS_TEXT,      0, g.text,           g.resetText           },
            { Pid::GLISS_SHOW_TEXT, 0, g.showText,       g.resetShowText       },
            { Pid::GLISSANDO_STYLE, 0, g.glissandoStyle, g.resetGlissandoStyle },
            { Pid::PLAY,            0, g.playGlissando,  g.resetPlayGlissando  },
            { Pid::FONT_FACE,       0, g.fontFace,       g.resetFontFace       },
            { Pid::FONT_SIZE,       0, g.fontSize,       g.resetFontSize       },
            { Pid::FONT_STYLE,      0, g.fontStyle,      g.resetFontStyle      },
            };
      const std::vector<InspectorPanel> ppList = {
            { g.title, g.panel }
            };
      mapSignals(iiList, ppList);
      }
}

