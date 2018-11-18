//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __INSPECTOR_MEASURENUMBER_H__
#define __INSPECTOR_MEASURENUMBER_H__

#include "inspector.h"
#include "inspectorTextBase.h"
#include "ui_inspector_text.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorMeasureNumber
//---------------------------------------------------------

class InspectorMeasureNumber : public InspectorTextBase {
      Q_OBJECT

   public:
      InspectorMeasureNumber(QWidget* parent);
      };

}

#endif

