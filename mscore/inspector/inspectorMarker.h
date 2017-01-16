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
#include "inspectorBase.h"
#include "ui_inspector_marker.h"
#include "libmscore/property.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorMarker
//---------------------------------------------------------

class InspectorMarker : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorText t;
      Ui::InspectorMarker  m;

   public:
      InspectorMarker(QWidget* parent);
      };


} // namespace Ms
#endif


