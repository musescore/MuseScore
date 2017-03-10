//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorOttava.h"
#include "inspectorTextLine.h"
#include "musescore.h"
#include "libmscore/ottava.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorOttava
//---------------------------------------------------------

InspectorOttava::InspectorOttava(QWidget* parent)
   : InspectorElementBase(parent)
      {
      l.setupUi(addWidget());
      setupLineStyle(l.lineStyle);
      tl.setupUi(addWidget());
      o.setupUi(addWidget());

      const std::vector<InspectorItem> il = {
            { P_ID::DIAGONAL,      0, l.diagonal,       l.resetDiagonal       },
            { P_ID::LINE_VISIBLE,  0, l.lineVisible,    l.resetLineVisible    },
            { P_ID::LINE_COLOR,    0, l.lineColor,      l.resetLineColor      },
            { P_ID::LINE_WIDTH,    0, l.lineWidth,      l.resetLineWidth      },
            { P_ID::LINE_STYLE,    0, l.lineStyle,      l.resetLineStyle      },
            { P_ID::DASH_LINE_LEN, 0, l.dashLineLength, l.resetDashLineLength },
            { P_ID::DASH_GAP_LEN,  0, l.dashGapLength,  l.resetDashGapLength  },

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

            { P_ID::OTTAVA_TYPE,             0, o.ottavaType,     o.resetOttavaType     },
            { P_ID::PLACEMENT,               0, o.placement,      o.resetPlacement      },
            { P_ID::NUMBERS_ONLY,            0, o.numbersOnly,    o.resetNumbersOnly    }
            };
      const std::vector<InspectorPanel> ppList = {
            { l.title,  l.panel },
            { tl.title, tl.panel },
            { o.title,  o.panel }
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
//   updateLineType
//---------------------------------------------------------

void InspectorOttava::updateLineType()
      {
      TextLineBaseSegment* ts = static_cast<TextLineBaseSegment*>(inspector->element());
      TextLineBase* t = ts->textLineBase();
      bool userDash = t->lineStyle() == Qt::CustomDashLine;

      l.dashLineLength->setVisible(userDash);
      l.dashGapLength->setVisible(userDash);
      l.resetDashLineLength->setVisible(userDash);
      l.resetDashGapLength->setVisible(userDash);
      l.dashLineLengthLabel->setVisible(userDash);
      l.dashGapLengthLabel->setVisible(userDash);
      }

//---------------------------------------------------------
//   updateBeginHookType
//---------------------------------------------------------

void InspectorOttava::updateBeginHookType()
      {
      TextLineBaseSegment* ts = static_cast<TextLineBaseSegment*>(inspector->element());
      TextLineBase* t = ts->textLineBase();
      bool hook = t->beginHookType() != HookType::NONE;

      tl.beginHookHeight->setVisible(hook);
      tl.resetBeginHookHeight->setVisible(hook);
      tl.beginHookHeightLabel->setVisible(hook);
      }

//---------------------------------------------------------
//   updateEndHookType
//---------------------------------------------------------

void InspectorOttava::updateEndHookType()
      {
      TextLineBaseSegment* ts = static_cast<TextLineBaseSegment*>(inspector->element());
      TextLineBase* t = ts->textLineBase();
      bool hook = t->endHookType() != HookType::NONE;

      tl.endHookHeight->setVisible(hook);
      tl.resetEndHookHeight->setVisible(hook);
      tl.endHookHeightLabel->setVisible(hook);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorOttava::setElement()
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

      updateLineType();
      updateBeginHookType();
      updateEndHookType();

      InspectorElementBase::setElement();
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorOttava::valueChanged(int idx)
      {
      InspectorBase::valueChanged(idx);
      if (iList[idx].t == P_ID::LINE_STYLE)
            updateLineType();
      else if (iList[idx].t == P_ID::BEGIN_HOOK_TYPE)
            updateBeginHookType();
      else if (iList[idx].t == P_ID::END_HOOK_TYPE)
            updateEndHookType();
      }


}

