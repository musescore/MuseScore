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

#ifndef __INSPECTOR_TEXT_LINE_BASE_H__
#define __INSPECTOR_TEXT_LINE_BASE_H__

#include "inspectorElementBase.h"
#include "ui_inspector_line.h"
#include "ui_inspector_textlinebase.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorTextBase
//---------------------------------------------------------

class InspectorTextLineBase : public InspectorElementBase {
      Q_OBJECT

      void updateBeginHookType();
      void updateEndHookType();
      void updateLineType();

   protected:
      Ui::InspectorLine l;
      Ui::InspectorTextLineBase tl;

   public:
      InspectorTextLineBase(QWidget* parent);
      virtual void setElement() override;
      virtual void valueChanged(int) override;
      };

} // namespace Ms


#endif



