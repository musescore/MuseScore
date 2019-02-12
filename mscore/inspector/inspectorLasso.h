//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __INSPECTOR_LASSO_H__
#define __INSPECTOR_LASSO_H__

#include "inspector.h"
#include "ui_inspector_lasso.h"
#include "libmscore/property.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorLasso
//---------------------------------------------------------

class InspectorLasso : public InspectorBase {
      Q_OBJECT

      Ui::InspectorLasso b;

   public:
      InspectorLasso(QWidget* parent);
      };


} // namespace Ms
#endif

