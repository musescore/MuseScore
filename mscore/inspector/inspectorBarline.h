//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __INSPECTOR_BARLINE_H__
#define __INSPECTOR_BARLINE_H__

#include "inspectorBase.h"
#include "inspectorElementBase.h"
#include "ui_inspector_barline.h"
#include "ui_inspector_segment.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorBarLine
//---------------------------------------------------------

class InspectorBarLine : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorSegment s;
      Ui::InspectorBarLine b;

      void  blockSpanDataSignals(bool val);

   private slots:
      void manageSpanData();
      void presetDefaultClicked();
      void presetTick1Clicked();
      void presetTick2Clicked();
      void presetShort1Clicked();
      void presetShort2Clicked();
      void setAsStaffDefault();

   public:
      InspectorBarLine(QWidget* parent);
      virtual void setElement() override;
      };

} // namespace Ms
#endif

