//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2013 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __INSPECTOR_PARTRANGE_H__
#define __INSPECTOR_PARTRANGE_H__

#include "inspector.h"
#include "inspectorBase.h"
#include "ui_inspector_ambitus.h"
#include "ui_inspector_segment.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorAmbitus
//---------------------------------------------------------

class InspectorAmbitus : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorAmbitus r;
      Ui::InspectorSegment s;

   public:
      InspectorAmbitus(QWidget* parent);

   protected slots:
      void updateRange();
      virtual void valueChanged(int idx) override;
      };


} // namespace Ms
#endif

