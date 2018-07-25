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

#ifndef __INSPECTOR_VOLTA_H__
#define __INSPECTOR_VOLTA_H__

#include "inspector.h"
#include "inspectorTextLineBase.h"
#include "ui_inspector_volta.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorVolta
//---------------------------------------------------------

class InspectorVolta : public InspectorTextLineBase {
      Q_OBJECT

      Ui::InspectorVolta v;

   public:
      InspectorVolta(QWidget* parent);
      };


} // namespace Ms
#endif

