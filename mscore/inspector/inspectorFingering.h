//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __INSPECTOR_FINGERING_H__
#define __INSPECTOR_FINGERING_H__

#include "inspector.h"
#include "ui_inspector_stafftext.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorFingering
//---------------------------------------------------------

class InspectorFingering : public InspectorStaffText {
      Q_OBJECT

   public:
      InspectorFingering(QWidget* parent);
      virtual const std::vector<Tid>& allowedTextStyles() override;
      };


} // namespace Ms
#endif
