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
            { Pid::DIAGONAL,      0, l.diagonal,       l.resetDiagonal       },
            { Pid::LINE_VISIBLE,  0, l.lineVisible,    l.resetLineVisible    },
            { Pid::LINE_COLOR,    0, l.lineColor,      l.resetLineColor      },
            { Pid::LINE_WIDTH,    0, l.lineWidth,      l.resetLineWidth      },
            { Pid::LINE_STYLE,    0, l.lineStyle,      l.resetLineStyle      },
            { Pid::DASH_LINE_LEN, 0, l.dashLineLength, l.resetDashLineLength },
            { Pid::DASH_GAP_LEN,  0, l.dashGapLength,  l.resetDashGapLength  },

            { Pid::BEGIN_TEXT,              0, tl.beginText,             tl.resetBeginText             },
            { Pid::BEGIN_TEXT_PLACE,        0, tl.beginTextPlacement,    tl.resetBeginTextPlacement    },
            { Pid::BEGIN_FONT_FACE,         0, tl.beginFontFace,         tl.resetBeginFontFace         },
            { Pid::BEGIN_FONT_SIZE,         0, tl.beginFontSize,         tl.resetBeginFontSize         },
            { Pid::BEGIN_FONT_BOLD,         0, tl.beginFontBold,         tl.resetBeginFontBold         },
            { Pid::BEGIN_FONT_ITALIC,       0, tl.beginFontItalic,       tl.resetBeginFontItalic       },
            { Pid::BEGIN_FONT_UNDERLINE,    0, tl.beginFontUnderline,    tl.resetBeginFontUnderline    },
            { Pid::BEGIN_TEXT_OFFSET,       0, tl.beginTextOffset,       tl.resetBeginTextOffset       },

            { Pid::BEGIN_HOOK_TYPE,         0, tl.beginHookType,         tl.resetBeginHookType         },
            { Pid::BEGIN_HOOK_HEIGHT,       0, tl.beginHookHeight,       tl.resetBeginHookHeight       },
            { Pid::CONTINUE_TEXT,           0, tl.continueText,          tl.resetContinueText          },
            { Pid::CONTINUE_TEXT_PLACE,     0, tl.continueTextPlacement, tl.resetContinueTextPlacement },
            { Pid::CONTINUE_FONT_FACE,      0, tl.continueFontFace,      tl.resetContinueFontFace      },
            { Pid::CONTINUE_FONT_SIZE,      0, tl.continueFontSize,      tl.resetContinueFontSize      },
            { Pid::CONTINUE_FONT_BOLD,      0, tl.continueFontBold,      tl.resetContinueFontBold      },
            { Pid::CONTINUE_FONT_ITALIC,    0, tl.continueFontItalic,    tl.resetContinueFontItalic    },
            { Pid::CONTINUE_FONT_UNDERLINE, 0, tl.continueFontUnderline, tl.resetContinueFontUnderline },
            { Pid::CONTINUE_TEXT_OFFSET,    0, tl.continueTextOffset,    tl.resetContinueTextOffset    },

            { Pid::END_TEXT,                0, tl.endText,               tl.resetEndText               },
            { Pid::END_TEXT_PLACE,          0, tl.endTextPlacement,      tl.resetEndTextPlacement      },
            { Pid::END_FONT_FACE,           0, tl.endFontFace,           tl.resetEndFontFace           },
            { Pid::END_FONT_SIZE,           0, tl.endFontSize,           tl.resetEndFontSize           },
            { Pid::END_FONT_BOLD,           0, tl.endFontBold,           tl.resetEndFontBold           },
            { Pid::END_FONT_ITALIC,         0, tl.endFontItalic,         tl.resetEndFontItalic         },
            { Pid::END_FONT_UNDERLINE,      0, tl.endFontUnderline,      tl.resetEndFontUnderline      },
            { Pid::END_TEXT_OFFSET,         0, tl.endTextOffset,         tl.resetEndTextOffset         },
            { Pid::END_HOOK_TYPE,           0, tl.endHookType,           tl.resetEndHookType           },
            { Pid::END_HOOK_HEIGHT,         0, tl.endHookHeight,         tl.resetEndHookHeight         },

            { Pid::OTTAVA_TYPE,             0, o.ottavaType,     o.resetOttavaType     },
            { Pid::PLACEMENT,               0, o.placement,      o.resetPlacement      },
            { Pid::NUMBERS_ONLY,            0, o.numbersOnly,    o.resetNumbersOnly    }
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
      if (iList[idx].t == Pid::LINE_STYLE)
            updateLineType();
      else if (iList[idx].t == Pid::BEGIN_HOOK_TYPE)
            updateBeginHookType();
      else if (iList[idx].t == Pid::END_HOOK_TYPE)
            updateEndHookType();
      }


}

