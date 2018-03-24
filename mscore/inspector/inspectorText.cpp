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
#include "icons.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorText
//---------------------------------------------------------

InspectorText::InspectorText(QWidget* parent)
   : InspectorTextBase(parent)
      {
      f.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { P_ID::SUB_STYLE, 0, f.subStyle,     f.resetSubStyle     },
            };

      const std::vector<InspectorPanel> ppList = {
            { f.title, f.panel }
            };

      f.subStyle->clear();
      for (auto ss : { SubStyleId::FRAME, SubStyleId::TITLE, SubStyleId::SUBTITLE,SubStyleId::COMPOSER,
         SubStyleId::POET, SubStyleId::INSTRUMENT_EXCERPT,
         SubStyleId::TRANSLATOR, SubStyleId::HEADER, SubStyleId::FOOTER, SubStyleId::USER1, SubStyleId::USER2 } )
            {
            f.subStyle->addItem(subStyleUserName(ss), int(ss));
            }

      mapSignals(iiList, ppList);
      }

}

