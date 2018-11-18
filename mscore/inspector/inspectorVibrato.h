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

#ifndef __INSPECTOR_VIBRATO_H__
#define __INSPECTOR_VIBRATO_H__

#include "inspector.h"
#include "inspectorBase.h"
#include "ui_inspector_vibrato.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorVibrato
//---------------------------------------------------------

class InspectorVibrato : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorVibrato v;

   public:
      InspectorVibrato(QWidget* parent);
      };


} // namespace Ms
#endif

