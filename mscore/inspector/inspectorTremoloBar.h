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

#ifndef __INSPECTOR_TREMOLO_BAR_H__
#define __INSPECTOR_TREMOLO_BAR_H__

#include "inspector.h"
#include "ui_inspector_tremolobar.h"
#include "libmscore/pitchvalue.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorTremoloBar
//---------------------------------------------------------

class InspectorTremoloBar : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorTremoloBar g;

   private slots:
      void tremoloBarTypeChanged(int);
      void updateTremoloBar();

   public:
      InspectorTremoloBar(QWidget* parent);
      virtual void setElement() override;
      };

}

#endif
