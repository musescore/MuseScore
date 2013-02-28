//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorGlissando.h"
#include "musescore.h"
#include "libmscore/glissando.h"
#include "libmscore/score.h"

//---------------------------------------------------------
//   InspectorGlissando
//---------------------------------------------------------

InspectorGlissando::InspectorGlissando(QWidget* parent)
   : InspectorBase(parent)
      {
      QWidget* w = new QWidget;
      b.setupUi(w);
      layout->addWidget(w);

      iList[0] = InspectorItem(P_COLOR,           b.color,    b.resetColor);
      iList[1] = InspectorItem(P_VISIBLE,         b.visible,  b.resetVisible);
      iList[2] = InspectorItem(P_USER_OFF, 0,     b.offsetX,  b.resetX);
      iList[3] = InspectorItem(P_USER_OFF, 1,     b.offsetY,  b.resetY);
      iList[4] = InspectorItem(P_GLISS_TYPE,      b.type,     b.resetType);
      iList[5] = InspectorItem(P_GLISS_TEXT,      b.text,     b.resetText);
      iList[6] = InspectorItem(P_GLISS_SHOW_TEXT, b.showText, b.resetShowText);

      mapSignals();
      }

