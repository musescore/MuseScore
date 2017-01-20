//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorFingering.h"
#include "musescore.h"
#include "libmscore/fingering.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorFingering
//---------------------------------------------------------

InspectorFingering::InspectorFingering(QWidget* parent)
   : InspectorElementBase(parent)
      {
      t.setupUi(addWidget());
      f.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { P_ID::FONT_FACE,        0, 0, t.fontFace,     t.resetFontFace     },
            { P_ID::FONT_SIZE,        0, 0, t.fontSize,     t.resetFontSize     },
            { P_ID::FONT_BOLD,        0, 0, t.bold,         t.resetBold         },
            { P_ID::FONT_ITALIC,      0, 0, t.italic,       t.resetItalic       },
            { P_ID::FONT_UNDERLINE,   0, 0, t.underline,    t.resetUnderline    },
            { P_ID::FRAME,            0, 0, t.hasFrame,     t.resetHasFrame     },
            { P_ID::FRAME_FG_COLOR,   0, 0, t.frameColor,   t.resetFrameColor   },
            { P_ID::FRAME_BG_COLOR,   0, 0, t.bgColor,      t.resetBgColor      },
            { P_ID::FRAME_CIRCLE,     0, 0, t.circle,       t.resetCircle       },
            { P_ID::FRAME_SQUARE,     0, 0, t.square,       t.resetSquare       },
            { P_ID::FRAME_WIDTH,      0, 0, t.frameWidth,   t.resetFrameWidth   },
            { P_ID::FRAME_PADDING,    0, 0, t.paddingWidth, t.resetPaddingWidth },
            { P_ID::FRAME_ROUND,      0, 0, t.frameRound,   t.resetFrameRound   },
            { P_ID::ALIGN,            0, 0, t.align,        t.resetAlign        },
            { P_ID::SUB_STYLE,        0, 0, f.subStyle,     f.resetSubStyle     },
            };
      const std::vector<InspectorPanel> ppList = {
            { t.title, t.panel },
            { f.title, f.panel }
            };

      f.subStyle->clear();
      for (auto ss : { SubStyle::FINGERING, SubStyle::LH_GUITAR_FINGERING, SubStyle::RH_GUITAR_FINGERING, SubStyle::STRING_NUMBER } )
            {
            f.subStyle->addItem(subStyleUserName(ss), int(ss));
            }

      mapSignals(iiList, ppList);
      }
}

