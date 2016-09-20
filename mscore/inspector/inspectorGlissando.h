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

#ifndef __INSPECTOR_GLISSANDO_H__
#define __INSPECTOR_GLISSANDO_H__

#include "inspector.h"
#include "ui_inspector_glissando.h"
#include "libmscore/property.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorGlissando
//---------------------------------------------------------

class InspectorGlissando : public InspectorElementBase {
      Q_OBJECT

//      UiInspectorElement   e;
      Ui::InspectorGlissando g;

   public:
      InspectorGlissando(QWidget* parent);
      };


} // namespace Ms
#endif
