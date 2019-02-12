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

#ifndef __INSPECTOR_TEXT_BASE_H__
#define __INSPECTOR_TEXT_BASE_H__

#include "inspectorElementBase.h"
#include "ui_inspector_text.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorTextBase
//---------------------------------------------------------

class InspectorTextBase : public InspectorElementBase {
      Q_OBJECT

      virtual void valueChanged(int, bool) override;
      void updateFrame();
      QComboBox* style;

   protected:
      Ui::InspectorText t;

   public:
      InspectorTextBase(QWidget* parent);
      virtual void setElement() override;
      virtual const std::vector<Tid>& allowedTextStyles();
      void populateStyle(QComboBox* style);
      };

} // namespace Ms


#endif



