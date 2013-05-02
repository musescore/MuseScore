//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
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

#include "inspectorBase.h"
#include "ui_inspector_element.h"
#include "ui_inspector_line.h"
#include "ui_inspector_trill.h"

//---------------------------------------------------------
//   InspectorTrill
//---------------------------------------------------------

class InspectorTrill : public InspectorBase {
      Q_OBJECT

      Ui::InspectorElement e;
      Ui::InspectorLine l;
      Ui::InspectorTrill t;

   public:
      InspectorTrill(QWidget* parent);
      };

#endif

