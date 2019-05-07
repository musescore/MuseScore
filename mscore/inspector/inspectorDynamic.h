//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __INSPECTOR_DYNAMIC_H__
#define __INSPECTOR_DYNAMIC_H__

#include "inspectorTextBase.h"
#include "ui_inspector_dynamic.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorDynamic
//---------------------------------------------------------

class InspectorDynamic : public InspectorTextBase {
      Q_OBJECT

      Ui::InspectorDynamic d;

      virtual void valueChanged(int, bool) override;

   public:
      InspectorDynamic(QWidget* parent);
      };

} // namespace Ms


#endif

