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

//---------------------------------------------------------
//   InspectorBeam
//---------------------------------------------------------

class InspectorBeam : public InspectorBase {
      Q_OBJECT

      Ui::InspectorBeam b;

      static const int _inspectorItems = 7;
      InspectorItem iList[_inspectorItems];

   protected slots:
      virtual void valueChanged(int idx);

   protected:
      virtual const InspectorItem& item(int idx) const { return iList[idx]; }
      virtual int inspectorItems() const { return _inspectorItems; }
      virtual void setValue(int idx, const QVariant& val);

   public:
      InspectorBeam(QWidget* parent);
      };

#endif

