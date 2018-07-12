//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __INSPECTOR_HARMONY_H__
#define __INSPECTOR_HARMONY_H__

#include "inspector.h"
#include "ui_inspector_harmony.h"
#include "libmscore/property.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorHarmony
//---------------------------------------------------------

class InspectorHarmony : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorHarmony b;

   protected slots:
      virtual void valueChanged(int idx) override;

   public:
      InspectorHarmony(QWidget* parent);
      };


} // namespace Ms
#endif

