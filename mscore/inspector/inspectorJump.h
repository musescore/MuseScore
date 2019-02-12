//=============================================================================
//  MuseScore
//  Music Composition & Notation
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

#include "inspectorTextBase.h"
#include "ui_inspector_jump.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorJump
//---------------------------------------------------------

class InspectorJump : public InspectorTextBase {
      Q_OBJECT

      Ui::InspectorJump j;

   public:
      InspectorJump(QWidget* parent);
      };


} // namespace Ms
#endif


