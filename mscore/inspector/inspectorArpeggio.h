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

#ifndef __INSPECTOR_ARPEGGIO_H__
#define __INSPECTOR_ARPEGGIO_H__

#include "inspector.h"
#include "ui_inspector_arpeggio.h"
#include "libmscore/property.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorArpeggio
//---------------------------------------------------------

class InspectorArpeggio : public InspectorElementBase {
      Q_OBJECT

//      UiInspectorElement   e;
      Ui::InspectorArpeggio g;

   public:
      InspectorArpeggio(QWidget* parent);
      };


} // namespace Ms
#endif
