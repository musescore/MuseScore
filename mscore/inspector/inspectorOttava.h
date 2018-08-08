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

#include "inspector.h"
#include "inspectorTextLineBase.h"
#include "ui_inspector_ottava.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorOttava
//---------------------------------------------------------

class InspectorOttava : public InspectorTextLineBase {
      Q_OBJECT

      Ui::InspectorOttava   o;

   public:
      InspectorOttava(QWidget* parent);
      };


} // namespace Ms
#endif

