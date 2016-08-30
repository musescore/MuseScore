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

#include "inspector.h"
#include "inspectorBase.h"
#include "ui_inspector_textline.h"
#include "ui_inspector_line.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorTextLine
//---------------------------------------------------------

class InspectorTextLine : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorLine l;
      Ui::InspectorTextLine tl;

   public:
      InspectorTextLine(QWidget* parent);
      };


} // namespace Ms
#endif

