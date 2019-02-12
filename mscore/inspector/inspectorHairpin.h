//=============================================================================
//  MuseScore
//  Music Composition & Notation
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
// #include "inspectorBase.h"
#include "inspectorTextLineBase.h"
#include "ui_inspector_hairpin.h"
#include "ui_inspector_line.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorHairpin
//---------------------------------------------------------

// class InspectorHairpin : public InspectorElementBase {
class InspectorHairpin : public InspectorTextLineBase {
      Q_OBJECT

//      Ui::InspectorLine l;
      Ui::InspectorHairpin h;

      void updateLineType();
      virtual void setElement() override;
      virtual void valueChanged(int idx) override;

   public:
      InspectorHairpin(QWidget* parent);
      virtual void postInit() override;
      };


} // namespace Ms
#endif

