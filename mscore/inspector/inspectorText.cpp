//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorText.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorText
//---------------------------------------------------------

InspectorText::InspectorText(QWidget* parent)
   : InspectorElementBase(parent)
      {
      t.setupUi(addWidget());

      iList.push_back({ P_ID::TEXT_STYLE_TYPE,    0, 0, t.style,    t.resetStyle    });

      connect(t.resetToStyle, SIGNAL(clicked()), SLOT(resetToStyle()));
      mapSignals();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorText::setElement()
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

