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

#ifndef __INSPECTOR_PEDAL_H__
#define __INSPECTOR_PEDAL_H__

#include "inspectorTextLineBase.h"
#include "ui_inspector_pedal.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorPedal
//---------------------------------------------------------

class InspectorPedal : public InspectorTextLineBase {
      Q_OBJECT

      Ui::InspectorPedal p;

   public:
      InspectorPedal(QWidget* parent);
      };

} // namespace Ms
#endif

