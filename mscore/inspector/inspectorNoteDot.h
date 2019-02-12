//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __INSPECTOR_NOTEDOT_H__
#define __INSPECTOR_NOTEDOT_H__

#include "inspector.h"
#include "inspectorBase.h"
#include "ui_inspector_notedot.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorNoteDot
//---------------------------------------------------------

class InspectorNoteDot : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorNoteDot    d;

   public:
      InspectorNoteDot(QWidget* parent);
      };


} // namespace Ms
#endif

