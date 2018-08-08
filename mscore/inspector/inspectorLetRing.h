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

#ifndef __INSPECTOR_LETRING_H__
#define __INSPECTOR_LETRING_H__

#include "inspectorTextLineBase.h"
#include "ui_inspector_letring.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorLetRing
//---------------------------------------------------------

class InspectorLetRing : public InspectorTextLineBase {
      Q_OBJECT

      Ui::InspectorLetRing lr;

   public:
      InspectorLetRing(QWidget* parent);
      };

} // namespace Ms
#endif

