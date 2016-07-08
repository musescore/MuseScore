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

#ifndef __INSPECTOR_HAIRPIN_H__
#define __INSPECTOR_HAIRPIN_H__

#include "inspector.h"
#include "inspectorBase.h"
#include "ui_inspector_hairpin.h"
#include "ui_inspector_line.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorHairpin
//---------------------------------------------------------

class InspectorHairpin : public InspectorBase {
      Q_OBJECT

      UiInspectorElement e;
      Ui::InspectorLine l;
      Ui::InspectorHairpin h;

   public:
      InspectorHairpin(QWidget* parent);
      virtual void postInit() override;
      virtual void setElement() override;
      };


} // namespace Ms
#endif

