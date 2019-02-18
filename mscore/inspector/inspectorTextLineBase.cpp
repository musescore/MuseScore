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
#include "inspectorTextLineBase.h"
#include "libmscore/textlinebase.h"
#include "icons.h"

namespace Ms {

//---------------------------------------------------------
//   populateHookType
//---------------------------------------------------------

void populateHookType(QComboBox* b)
      {
      b->clear();
      b->addItem(b->QObject::tr("None"), int(HookType::NONE));
      b->addItem(b->QObject::tr("90\u00b0"), int(HookType::HOOK_90)); // &deg;
      b->addItem(b->QObject::tr("45\u00b0"), int(HookType::HOOK_45)); // &deg;
      b->addItem(b->QObject::tr("90\u00b0 centered"), int(HookType::HOOK_90T)); // &deg;
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
//   InspectorTextLineBase
//---------------------------------------------------------

InspectorTextLineBase::InspectorTextLineBase(QWidget* parent)
   : InspectorElementBase(parent)
      {
      l.setupUi(addWidget());
      setupLineStyle(l.lineStyle);
      tl.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::DIAGONAL,                0, l.diagonal,                l.resetDiagonal              },
            { Pid::LINE_VISIBLE,            0, l.lineVisible,             l.resetLineVisible           },
            { Pid::COLOR,                   0, l.lineColor,               l.resetLineColor             },
            { Pid::LINE_WIDTH,              0, l.lineWidth,               l.resetLineWidth             },
            { Pid::LINE_STYLE,              0, l.lineStyle,               l.resetLineStyle             },
            { Pid::DASH_LINE_LEN,           0, l.dashLineLength,          l.resetDashLineLength        },
            { Pid::DASH_GAP_LEN,            0, l.dashGapLength,           l.resetDashGapLength         },

            { Pid::BEGIN_TEXT,              0, tl.beginText,             tl.resetBeginText             },
            { Pid::BEGIN_TEXT_PLACE,        0, tl.beginTextPlacement,    tl.resetBeginTextPlacement    },
            { Pid::BEGIN_TEXT_ALIGN,        0, tl.beginTextAlign,        tl.resetBeginTextAlign        },
            { Pid::BEGIN_FONT_FACE,         0, tl.beginFontFace,         tl.resetBeginFontFace         },
            { Pid::BEGIN_FONT_SIZE,         0, tl.beginFontSize,         tl.resetBeginFontSize         },
            { Pid::BEGIN_FONT_STYLE,        0, tl.beginFontStyle,        tl.resetBeginFontStyle        },
            { Pid::BEGIN_TEXT_OFFSET,       0, tl.beginTextOffset,       tl.resetBeginTextOffset       },

            { Pid::BEGIN_HOOK_TYPE,         0, tl.beginHookType,         tl.resetBeginHookType         },
            { Pid::BEGIN_HOOK_HEIGHT,       0, tl.beginHookHeight,       tl.resetBeginHookHeight       },
            { Pid::CONTINUE_TEXT,           0, tl.continueText,          tl.resetContinueText          },
            { Pid::CONTINUE_TEXT_PLACE,     0, tl.continueTextPlacement, tl.resetContinueTextPlacement },
            { Pid::CONTINUE_TEXT_ALIGN,     0, tl.continueTextAlign,     tl.resetContinueTextAlign     },
            { Pid::CONTINUE_FONT_FACE,      0, tl.continueFontFace,      tl.resetContinueFontFace      },
            { Pid::CONTINUE_FONT_SIZE,      0, tl.continueFontSize,      tl.resetContinueFontSize      },
            { Pid::CONTINUE_FONT_STYLE,     0, tl.continueFontStyle,     tl.resetContinueFontStyle     },
            { Pid::CONTINUE_TEXT_OFFSET,    0, tl.continueTextOffset,    tl.resetContinueTextOffset    },

            { Pid::END_TEXT,                0, tl.endText,               tl.resetEndText               },
            { Pid::END_TEXT_PLACE,          0, tl.endTextPlacement,      tl.resetEndTextPlacement      },
            { Pid::END_TEXT_ALIGN,          0, tl.endTextAlign,          tl.resetEndTextAlign          },
            { Pid::END_FONT_FACE,           0, tl.endFontFace,           tl.resetEndFontFace           },
            { Pid::END_FONT_SIZE,           0, tl.endFontSize,           tl.resetEndFontSize           },
            { Pid::END_FONT_STYLE,          0, tl.endFontStyle,          tl.resetEndFontStyle          },
            { Pid::END_TEXT_OFFSET,         0, tl.endTextOffset,         tl.resetEndTextOffset         },
            { Pid::END_HOOK_TYPE,           0, tl.endHookType,           tl.resetEndHookType           },
            { Pid::END_HOOK_HEIGHT,         0, tl.endHookHeight,         tl.resetEndHookHeight         },
            };

      for (auto& i : iiList)
            iList.push_back(i);
      const std::vector<InspectorPanel> ppList = {
            { l.title,  l.panel },
            { tl.title, tl.panel },
            };
      for (auto& i : ppList)
            pList.push_back(i);

      populateHookType(tl.beginHookType);
      populateHookType(tl.endHookType);
      populateTextPlace(tl.beginTextPlacement);
      populateTextPlace(tl.continueTextPlacement);
      populateTextPlace(tl.endTextPlacement);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorTextLineBase::setElement()
      {
      InspectorElementBase::setElement();
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
      }

//---------------------------------------------------------
//   updateLineType
//---------------------------------------------------------

void InspectorTextLineBase::updateLineType()
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

void InspectorTextLineBase::updateBeginHookType()
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

void InspectorTextLineBase::updateEndHookType()
      {
      TextLineBaseSegment* ts = static_cast<TextLineBaseSegment*>(inspector->element());
      TextLineBase* t = ts->textLineBase();
      bool hook = t->endHookType() != HookType::NONE;

      tl.endHookHeight->setVisible(hook);
      tl.resetEndHookHeight->setVisible(hook);
      tl.endHookHeightLabel->setVisible(hook);
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorTextLineBase::valueChanged(int idx)
      {
      InspectorBase::valueChanged(idx);
      if (iList[idx].t == Pid::LINE_STYLE)
            updateLineType();
      else if (iList[idx].t == Pid::BEGIN_HOOK_TYPE)
            updateBeginHookType();
      else if (iList[idx].t == Pid::END_HOOK_TYPE)
            updateEndHookType();
      else if (iList[idx].t == Pid::COLOR)
            inspector->update();
      }


} // namespace Ms

