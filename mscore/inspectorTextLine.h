//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __INSPECTOR_TEXTLINE_H__
#define __INSPECTOR_TEXTLINE_H__

#include "inspectorBase.h"
#include "ui_inspector_element.h"
#include "ui_inspector_textline.h"

//---------------------------------------------------------
//   InspectorHairpin
//---------------------------------------------------------

class InspectorTextLine : public InspectorBase {
      Q_OBJECT

      Ui::InspectorElement e;
      Ui::InspectorTextLine l;

   public:
      InspectorTextLine(QWidget* parent);
      };

#endif

