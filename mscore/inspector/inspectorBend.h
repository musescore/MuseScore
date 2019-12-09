//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __INSPECTOR_BEND_H__
#define __INSPECTOR_BEND_H__

#include "inspector.h"
#include "ui_inspector_bend.h"
#include "libmscore/pitchvalue.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorBend
//---------------------------------------------------------

class InspectorBend : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorBend g;

   private slots:
      void bendTypeChanged(int);
      void updateBend();

   public:
      InspectorBend(QWidget* parent);
      const QList<PitchValue>& points() const;
      virtual void setElement() override;
      };

static const QList<PitchValue> BEND 
   = { PitchValue(0, 0),   PitchValue(15, 100), PitchValue(60, 100) };
static const QList<PitchValue> BEND_RELEASE
   = { PitchValue(0, 0),   PitchValue(10, 100), PitchValue(20, 100), PitchValue(30, 0), PitchValue(60, 0) };
static const QList<PitchValue> BEND_RELEASE_BEND
   = { PitchValue(0, 0),   PitchValue(10, 100), PitchValue(20, 100), PitchValue(30, 0), PitchValue(40, 0), PitchValue(50, 100), PitchValue(60, 100) };
static const QList<PitchValue> PREBEND
   = { PitchValue(0, 100), PitchValue(60, 100) };
static const QList<PitchValue> PREBEND_RELEASE
   = { PitchValue(0, 100), PitchValue(15, 100), PitchValue(30, 0),   PitchValue(60, 0) };

} // namespace Ms
#endif
