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
   : InspectorElementBase(parent)
      {
      t.setupUi(addWidget());
      j.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { P_ID::TEXT_STYLE_TYPE,    0, 0,     t.style,      t.resetStyle      },
            { P_ID::JUMP_TO,            0, false, j.jumpTo,     j.resetJumpTo     },
            { P_ID::PLAY_UNTIL,         0, false, j.playUntil,  j.resetPlayUntil  },
            { P_ID::CONTINUE_AT,        0, false, j.continueAt, j.resetContinueAt }
            };

      mapSignals(iiList);
      connect(t.resetToStyle, SIGNAL(clicked()), SLOT(resetToStyle()));
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
                  t.style->addItem(qApp->translate("TextStyle",ts.at(i).name().toUtf8().data()), i);
            }
      t.style->blockSignals(false);
      InspectorElementBase::setElement();
      }

}
