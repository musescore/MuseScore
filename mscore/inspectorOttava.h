//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __INSPECTOR_OTTAVA_H__
#define __INSPECTOR_OTTAVA_H__

#include "inspectorBase.h"
#include "ui_inspector_element.h"
#include "ui_inspector_textline.h"
#include "ui_inspector_ottava.h"

//---------------------------------------------------------
//   InspectorOttava
//---------------------------------------------------------

class InspectorOttava : public InspectorBase {
      Q_OBJECT

      Ui::InspectorElement  e;
      Ui::InspectorTextLine l;
      Ui::InspectorOttava   o;

   public:
      InspectorOttava(QWidget* parent);
      };

#endif

