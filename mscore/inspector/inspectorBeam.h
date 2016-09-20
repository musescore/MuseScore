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

#ifndef __INSPECTOR_BEAM_H__
#define __INSPECTOR_BEAM_H__

#include "inspector.h"
#include "ui_inspector_beam.h"
#include "libmscore/property.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorBeam
//---------------------------------------------------------

class InspectorBeam : public InspectorElementBase {
      Q_OBJECT

//      UiInspectorElement e;
      Ui::InspectorBeam b;

   protected slots:
      virtual void valueChanged(int idx) override;

   protected:
      virtual void setValue(const InspectorItem&, QVariant val) override;

   public:
      InspectorBeam(QWidget* parent);
      };


} // namespace Ms
#endif

