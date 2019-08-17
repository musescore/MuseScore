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

#ifndef __INSPECTOR_ELEMENT_BASE_H__
#define __INSPECTOR_ELEMENT_BASE_H__

#include "ui_inspector_element.h"
#include "inspectorBase.h"

namespace Ms {

//---------------------------------------------------------
//   UiInspectorElement
//---------------------------------------------------------

class UiInspectorElement: public Ui::InspectorElement {
   public:
      void setupUi(QWidget *InspectorElement);
      };

//---------------------------------------------------------
//   InspectorElementBase
//---------------------------------------------------------

class InspectorElementBase : public InspectorBase {
      Q_OBJECT

   protected:
      UiInspectorElement e;

   private slots:

   public:
      InspectorElementBase(QWidget* parent);
      virtual void setElement() override;
      };

} // namespace Ms


#endif


