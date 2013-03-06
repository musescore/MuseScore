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

#ifndef __INSPECTOR_JUMP_H__
#define __INSPECTOR_JUMP_H__

#include "inspector.h"
#include "ui_inspector_jump.h"
#include "libmscore/property.h"

//---------------------------------------------------------
//   InspectorJump
//---------------------------------------------------------

class InspectorJump : public InspectorBase {
      Q_OBJECT

      Ui::InspectorJump b;

   public:
      InspectorJump(QWidget* parent);
      };

#endif


