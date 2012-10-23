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

#ifndef __INSPECTOR_OTTAVA_H__
#define __INSPECTOR_OTTAVA_H__

#include "inspector.h"
#include "ui_inspector_ottava.h"
#include "libmscore/property.h"

//---------------------------------------------------------
//   InspectorOttava
//---------------------------------------------------------

class InspectorOttava : public InspectorBase {
      Q_OBJECT

      InspectorElementElement* iElement;
      Ui::InspectorOttava iOttava;

   public slots:
      virtual void apply();

   public:
      InspectorOttava(QWidget* parent);
      virtual void setElement(Element*);
      bool dirty() const;
      };

#endif

