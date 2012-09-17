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

#include "inspector.h"
#include "ui_inspector_trill.h"
#include "libmscore/property.h"

//---------------------------------------------------------
//   InspectorTrill
//---------------------------------------------------------

class InspectorTrill : public InspectorBase {
      Q_OBJECT

      InspectorElementElement* iElement;
      Ui::InspectorTrill iTrill;

   public slots:
      virtual void apply();

   public:
      InspectorTrill(QWidget* parent);
      virtual void setElement(Element*);
      bool dirty() const;
      };

#endif

