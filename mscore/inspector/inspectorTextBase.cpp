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
#include "libmscore/score.h"
#include "icons.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorTextBase
//---------------------------------------------------------

InspectorTextBase::InspectorTextBase(QWidget* parent)
   : InspectorElementBase(parent)
      {
      t.setupUi(addWidget());
      style = nullptr;

      const std::vector<InspectorItem> iiList = {
            { Pid::FONT_FACE,         0, t.fontFace,     t.resetFontFace     },
            { Pid::FONT_SIZE,         0, t.fontSize,     t.resetFontSize     },
            { Pid::SIZE_SPATIUM_DEPENDENT,        0,  t.spatiumDependent,    t.resetSpatiumDependent    },
            { Pid::FONT_STYLE,        0, t.fontStyle,    t.resetFontStyle    },
            { Pid::FRAME_TYPE,        0, t.frameType,    t.resetFrameType    },
            { Pid::FRAME_FG_COLOR,    0, t.frameColor,   t.resetFrameColor   },
            { Pid::FRAME_BG_COLOR,    0, t.bgColor,      t.resetBgColor      },
            { Pid::FRAME_WIDTH,       0, t.frameWidth,   t.resetFrameWidth   },
            { Pid::FRAME_PADDING,     0, t.paddingWidth, t.resetPaddingWidth },
            { Pid::FRAME_ROUND,       0, t.frameRound,   t.resetFrameRound   },
            { Pid::ALIGN,             0, t.align,        t.resetAlign        },
            };
      for (auto& i : iiList)
            iList.push_back(i);
      const std::vector<InspectorPanel> ppList = {
            { t.title, t.panel },
            };
      for (auto& i : ppList)
            pList.push_back(i);
      QComboBox* b = t.frameType;
      b->clear();
      b->addItem(b->QObject::tr("None"), int(FrameType::NO_FRAME));
      b->addItem(b->QObject::tr("Square"), int(FrameType::SQUARE));
      b->addItem(b->QObject::tr("Circle"), int(FrameType::CIRCLE));

      connect(t.resetToStyle, SIGNAL(clicked()), SLOT(resetToStyle()));
      }

//---------------------------------------------------------
//   updateFrame
//---------------------------------------------------------

void InspectorTextBase::updateFrame()
      {
      TextBase* text = toTextBase(inspector->element());
      t.frameWidget->setVisible(text->hasFrame());
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorTextBase::valueChanged(int idx, bool b)
      {
      InspectorElementBase::valueChanged(idx, b);
      Pid pid = iList[idx].t;
      if (pid == Pid::FRAME_TYPE)
            updateFrame();
      TextBase* text = toTextBase(inspector->element());
      t.resetToStyle->setEnabled(text->hasCustomFormatting());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorTextBase::setElement()
      {
      InspectorElementBase::setElement();
      updateFrame();
      TextBase* text = toTextBase(inspector->element());
      t.resetToStyle->setEnabled(text->hasCustomFormatting());
      populateStyle(style);
      }

//---------------------------------------------------------
//   allowedTextStyles
//---------------------------------------------------------

const std::vector<Tid>& InspectorTextBase::allowedTextStyles()
      {
      return primaryTextStyles();
      }

//---------------------------------------------------------
//   populateStyle
//---------------------------------------------------------

void InspectorTextBase::populateStyle(QComboBox* s)
      {
      if (s) {
            this->style = s;
            Score* score = inspector->element()->score();
            s->blockSignals(true);
            int idx = s->currentIndex();
            s->clear();
            for (auto ss : allowedTextStyles())
                  s->addItem(score->getTextStyleUserName(ss), int(ss));
            s->setCurrentIndex(idx);
            s->blockSignals(false);
           }
      }

} // namespace Ms

