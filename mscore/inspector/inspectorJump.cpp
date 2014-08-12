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

#include "inspectorJump.h"
#include "musescore.h"
#include "libmscore/jump.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorJump
//---------------------------------------------------------

InspectorJump::InspectorJump(QWidget* parent)
   : InspectorBase(parent)
      {
      b.setupUi(addWidget());
      t.setupUi(addWidget());
      j.setupUi(addWidget());

      iList = {
            { P_ID::COLOR,              0, false, b.color,      b.resetColor      },
            { P_ID::VISIBLE,            0, false, b.visible,    b.resetVisible    },
            { P_ID::USER_OFF,           0, false, b.offsetX,    b.resetX          },
            { P_ID::USER_OFF,           1, false, b.offsetY,    b.resetY          },
            { P_ID::TEXT_STYLE_TYPE,    0, 0,     t.style,      t.resetStyle      },
            { P_ID::JUMP_TO,            0, false, j.jumpTo,     j.resetJumpTo     },
            { P_ID::PLAY_UNTIL,         0, false, j.playUntil,  j.resetPlayUntil  },
            { P_ID::CONTINUE_AT,        0, false, j.continueAt, j.resetContinueAt }
            };

      mapSignals();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorJump::setElement()
      {
      Element* e = inspector->element();
      Score* score = e->score();

      t.style->blockSignals(true);
      t.style->clear();
      const QList<TextStyle>& ts = score->style()->textStyles();
      int n = ts.size();
      for (int i = 0; i < n; ++i) {
            if (!(ts.at(i).hidden() & TextStyleHidden::IN_LISTS) )
                  t.style->addItem(qApp->translate("TextStyle",ts.at(i).name().toLatin1().data()), i);
            }
      t.style->blockSignals(false);
      InspectorBase::setElement();
      }

}
