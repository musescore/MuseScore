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

//---------------------------------------------------------
//   InspectorGlissando
//---------------------------------------------------------

class InspectorGlissando : public InspectorBase {
      Q_OBJECT

      Ui::InspectorGlissando b;

      static const int _inspectorItems = 7;
      InspectorItem iList[_inspectorItems];

   protected:
      virtual const InspectorItem& item(int idx) const { return iList[idx];      }
      virtual int inspectorItems() const               { return _inspectorItems; }

   public:
      InspectorGlissando(QWidget* parent);
      };

#endif


