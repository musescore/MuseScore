//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspector.h"
#include "inspectorTextBase.h"
#include "libmscore/text.h"
#include "icons.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorTextBase
//---------------------------------------------------------

InspectorTextBase::InspectorTextBase(QWidget* parent)
   : InspectorElementBase(parent)
      {
      t.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { P_ID::FONT_FACE,         0, t.fontFace,     t.resetFontFace     },
            { P_ID::FONT_SIZE,         0, t.fontSize,     t.resetFontSize     },
            { P_ID::FONT_BOLD,         0, t.bold,         t.resetBold         },
            { P_ID::FONT_ITALIC,       0, t.italic,       t.resetItalic       },
            { P_ID::FONT_UNDERLINE,    0, t.underline,    t.resetUnderline    },
            { P_ID::FRAME,             0, t.hasFrame,     t.resetHasFrame     },
            { P_ID::FRAME_FG_COLOR,    0, t.frameColor,   t.resetFrameColor   },
            { P_ID::FRAME_BG_COLOR,    0, t.bgColor,      t.resetBgColor      },
            { P_ID::FRAME_CIRCLE,      0, t.circle,       t.resetCircle       },
            { P_ID::FRAME_SQUARE,      0, t.square,       t.resetSquare       },
            { P_ID::FRAME_WIDTH,       0, t.frameWidth,   t.resetFrameWidth   },
            { P_ID::FRAME_PADDING,     0, t.paddingWidth, t.resetPaddingWidth },
            { P_ID::FRAME_ROUND,       0, t.frameRound,   t.resetFrameRound   },
            { P_ID::ALIGN,             0, t.align,        t.resetAlign        },
            };
      for (auto& i : iiList)
            iList.push_back(i);
      const std::vector<InspectorPanel> ppList = {
            { t.title, t.panel },
            };
      for (auto& i : ppList)
            pList.push_back(i);
      t.bold->setIcon(*icons[int(Icons::textBold_ICON)]);
      t.underline->setIcon(*icons[int(Icons::textUnderline_ICON)]);
      t.italic->setIcon(*icons[int(Icons::textItalic_ICON)]);

      connect(t.resetToStyle, SIGNAL(clicked()), SLOT(resetToStyle()));
      }

//---------------------------------------------------------
//   updateFrame
//---------------------------------------------------------

void InspectorTextBase::updateFrame()
      {
      Text* text = static_cast<Text*>(inspector->element());
      t.frameWidget->setVisible(text->hasFrame());
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorTextBase::valueChanged(int idx)
      {
      InspectorElementBase::valueChanged(idx);
      if (iList[idx].t == P_ID::FRAME)
            updateFrame();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorTextBase::setElement()
      {
      InspectorElementBase::setElement();
      updateFrame();
      }

} // namespace Ms

