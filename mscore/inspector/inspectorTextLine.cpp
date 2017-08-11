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

#include "inspectorTextLine.h"
#include "musescore.h"
#include "libmscore/textline.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   populateHookType
//---------------------------------------------------------

void populateHookType(QComboBox* b)
      {
      b->clear();
      b->addItem(b->QObject::tr("None"), int(HookType::NONE));
      b->addItem(b->QObject::tr("90°"), int(HookType::HOOK_90));
      b->addItem(b->QObject::tr("45°"), int(HookType::HOOK_45));
      }

//---------------------------------------------------------
//   populateTextPlace
//---------------------------------------------------------

void populateTextPlace(QComboBox* b)
      {
      b->clear();
      b->addItem(b->QObject::tr("Auto"),  int(PlaceText::AUTO));
      b->addItem(b->QObject::tr("Above"), int(PlaceText::ABOVE));
      b->addItem(b->QObject::tr("Below"), int(PlaceText::BELOW));
      b->addItem(b->QObject::tr("Left"),  int(PlaceText::LEFT));
      }

//---------------------------------------------------------
//   InspectorTextLine
//---------------------------------------------------------

InspectorTextLine::InspectorTextLine(QWidget* parent)
   : InspectorElementBase(parent)
      {
      l.setupUi(addWidget());
      setupLineStyle(l.lineStyle);
      tl.setupUi(addWidget());

      const std::vector<InspectorItem> il = {
            { P_ID::DIAGONAL,                0, l.diagonal,               l.resetDiagonal               },
            { P_ID::LINE_VISIBLE,            0, l.lineVisible,            l.resetLineVisible            },
            { P_ID::LINE_COLOR,              0, l.lineColor,              l.resetLineColor              },
            { P_ID::LINE_WIDTH,              0, l.lineWidth,              l.resetLineWidth              },
            { P_ID::LINE_STYLE,              0, l.lineStyle,              l.resetLineStyle              },
            { P_ID::DASH_LINE_LEN,           0, l.dashLineLength,         l.resetDashLineLength         },
            { P_ID::DASH_GAP_LEN,            0, l.dashGapLength,          l.resetDashGapLength          },

            { P_ID::BEGIN_TEXT,              0, tl.beginText,             tl.resetBeginText             },
            { P_ID::BEGIN_TEXT_PLACE,        0, tl.beginTextPlacement,    tl.resetBeginTextPlacement    },
            { P_ID::BEGIN_FONT_FACE,         0, tl.beginFontFace,         tl.resetBeginFontFace         },
            { P_ID::BEGIN_FONT_SIZE,         0, tl.beginFontSize,         tl.resetBeginFontSize         },
            { P_ID::BEGIN_FONT_BOLD,         0, tl.beginFontBold,         tl.resetBeginFontBold         },
            { P_ID::BEGIN_FONT_ITALIC,       0, tl.beginFontItalic,       tl.resetBeginFontItalic       },
            { P_ID::BEGIN_FONT_UNDERLINE,    0, tl.beginFontUnderline,    tl.resetBeginFontUnderline    },
            { P_ID::BEGIN_TEXT_OFFSET,       0, tl.beginTextOffset,       tl.resetBeginTextOffset       },

            { P_ID::BEGIN_HOOK_TYPE,         0, tl.beginHookType,         tl.resetBeginHookType         },
            { P_ID::BEGIN_HOOK_HEIGHT,       0, tl.beginHookHeight,       tl.resetBeginHookHeight       },
            { P_ID::CONTINUE_TEXT,           0, tl.continueText,          tl.resetContinueText          },
            { P_ID::CONTINUE_TEXT_PLACE,     0, tl.continueTextPlacement, tl.resetContinueTextPlacement },
            { P_ID::CONTINUE_FONT_FACE,      0, tl.continueFontFace,      tl.resetContinueFontFace      },
            { P_ID::CONTINUE_FONT_SIZE,      0, tl.continueFontSize,      tl.resetContinueFontSize      },
            { P_ID::CONTINUE_FONT_BOLD,      0, tl.continueFontBold,      tl.resetContinueFontBold      },
            { P_ID::CONTINUE_FONT_ITALIC,    0, tl.continueFontItalic,    tl.resetContinueFontItalic    },
            { P_ID::CONTINUE_FONT_UNDERLINE, 0, tl.continueFontUnderline, tl.resetContinueFontUnderline },
            { P_ID::CONTINUE_TEXT_OFFSET,    0, tl.continueTextOffset,    tl.resetContinueTextOffset    },

            { P_ID::END_TEXT,                0, tl.endText,               tl.resetEndText               },
            { P_ID::END_TEXT_PLACE,          0, tl.endTextPlacement,      tl.resetEndTextPlacement      },
            { P_ID::END_FONT_FACE,           0, tl.endFontFace,           tl.resetEndFontFace           },
            { P_ID::END_FONT_SIZE,           0, tl.endFontSize,           tl.resetEndFontSize           },
            { P_ID::END_FONT_BOLD,           0, tl.endFontBold,           tl.resetEndFontBold           },
            { P_ID::END_FONT_ITALIC,         0, tl.endFontItalic,         tl.resetEndFontItalic         },
            { P_ID::END_FONT_UNDERLINE,      0, tl.endFontUnderline,      tl.resetEndFontUnderline      },
            { P_ID::END_TEXT_OFFSET,         0, tl.endTextOffset,         tl.resetEndTextOffset         },
            { P_ID::END_HOOK_TYPE,           0, tl.endHookType,           tl.resetEndHookType           },
            { P_ID::END_HOOK_HEIGHT,         0, tl.endHookHeight,         tl.resetEndHookHeight         },
            };
      const std::vector<InspectorPanel> ppList = {
            { l.title,  l.panel  },
            { tl.title, tl.panel }
            };
      populateHookType(tl.beginHookType);
      populateHookType(tl.endHookType);
      populateTextPlace(tl.beginTextPlacement);
      populateTextPlace(tl.continueTextPlacement);
      populateTextPlace(tl.endTextPlacement);

      tl.beginWidget->setVisible(false);
      tl.continueWidget->setVisible(false);
      tl.endWidget->setVisible(false);

      mapSignals(il, ppList);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorTextLine::setElement()
      {
      TextLineBaseSegment* ts = static_cast<TextLineBaseSegment*>(inspector->element());
      TextLineBase* t = ts->textLineBase();

      bool bt = !t->beginText().isEmpty();
      bool ct = !t->continueText().isEmpty();
      bool et = !t->endText().isEmpty();

      tl.hasBeginText->setChecked(bt);
      tl.hasContinueText->setChecked(ct);
      tl.hasEndText->setChecked(et);

      tl.beginWidget->setVisible(bt);
      tl.continueWidget->setVisible(ct);
      tl.endWidget->setVisible(et);

      InspectorElementBase::setElement();
      }

}

