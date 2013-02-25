//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2013 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __INSPECTOR_MARKER_H__
#define __INSPECTOR_MARKER_H__

#include "inspector.h"
#include "ui_inspector_marker.h"
#include "libmscore/property.h"

//---------------------------------------------------------
//   InspectorMarker
//---------------------------------------------------------

class InspectorMarker : public InspectorBase {
      Q_OBJECT

      InspectorElementElement* iElement;
      Ui::InspectorMarker iMarker;

   public slots:
      virtual void apply();

   public:
      InspectorMarker(QWidget* parent);
      virtual void setElement(Element*);
      bool dirty() const;
      };

#endif


