//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorBeam.h"
#include "musescore.h"
#include "libmscore/beam.h"
#include "libmscore/score.h"

//---------------------------------------------------------
//   InspectorBeam
//---------------------------------------------------------

InspectorBeam::InspectorBeam(QWidget* parent)
   : InspectorBase(parent)
      {
      QWidget* w = new QWidget;
      b.setupUi(w);
      layout->addWidget(w);

      iList[0].t = P_STEM_DIRECTION;
      iList[0].w = b.direction;
      iList[0].r = b.resetDirection;

      iList[1].t = P_DISTRIBUTE;
      iList[1].w = b.distribute;
      iList[1].r = b.resetDistribute;

      iList[2].t = P_GROW_LEFT;
      iList[2].w = b.growLeft;
      iList[2].r = b.resetGrowLeft;

      iList[3].t = P_GROW_RIGHT;
      iList[3].w = b.growRight;
      iList[3].r = b.resetGrowRight;

      iList[4].t = P_USER_MODIFIED;
      iList[4].w = b.userPosition;
      iList[4].r = b.resetUserPosition;

      iList[5].t = P_BEAM_POS;
      iList[5].sv = 0;
      iList[5].w = b.y1;
      iList[5].r = 0;

      iList[6].t = P_BEAM_POS;
      iList[6].sv = 1;
      iList[6].w = b.y2;
      iList[6].r = 0;

      mapSignals();
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorBeam::valueChanged(int idx)
      {
      if (idx == 4) {
            bool val = getValue(idx).toBool();
            iList[5].w->setEnabled(val);
            iList[6].w->setEnabled(val);
            }
      InspectorBase::valueChanged(idx);
      }

void InspectorBeam::setValue(int idx, const QVariant& val)
      {
      if (idx == 4) {
            bool enable = val.toBool();
            iList[5].w->setEnabled(enable);
            iList[6].w->setEnabled(enable);
            }
      InspectorBase::setValue(idx, val);
      }

