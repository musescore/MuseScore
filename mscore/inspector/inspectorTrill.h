//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __INSPECTOR_TRILL_H__
#define __INSPECTOR_TRILL_H__

#include "inspector.h"
#include "inspectorBase.h"
#include "ui_inspector_line.h"
#include "ui_inspector_trill.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorTrill
//---------------------------------------------------------

class InspectorTrill : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorTrill t;

   public:
      InspectorTrill(QWidget* parent);
      };


} // namespace Ms
#endif

