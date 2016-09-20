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

namespace Ms {

//---------------------------------------------------------
//   InspectorBeam
//---------------------------------------------------------

InspectorBeam::InspectorBeam(QWidget* parent)
   : InspectorElementBase(parent)
      {
      b.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { P_ID::STEM_DIRECTION, 0, false, b.direction,    b.resetDirection    },
            { P_ID::DISTRIBUTE,     0, false, b.distribute,   b.resetDistribute   },
            { P_ID::GROW_LEFT,      0, false, b.growLeft,     b.resetGrowLeft     },
            { P_ID::GROW_RIGHT,     0, false, b.growRight,    b.resetGrowRight    },
            { P_ID::BEAM_NO_SLOPE,  0, false, b.noSlope,      b.resetNoSlope      },
            { P_ID::USER_MODIFIED,  0, false, b.userPosition, b.resetUserPosition },
            { P_ID::BEAM_POS,       0, false, b.y1,           0                   },
            { P_ID::BEAM_POS,       1, false, b.y2,           0                   }
            };
      mapSignals(iiList);
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorBeam::valueChanged(int idx)
      {
      if (iList[idx].t == P_ID::USER_MODIFIED) {
            bool val = getValue(iList[idx]).toBool();
            b.noSlope->setEnabled(!val);
            b.y1->setEnabled(val);
            b.y2->setEnabled(val);
            }
      else if (iList[idx].t == P_ID::BEAM_NO_SLOPE) {
            bool val = getValue(iList[idx]).toBool();
            b.userPosition->setEnabled(!val);
            b.y1->setEnabled(!val);
            b.y2->setEnabled(!val);
            }
      InspectorElementBase::valueChanged(idx);
      }

void InspectorBeam::setValue(const InspectorItem& ii, QVariant val)
      {
      if (ii.w == b.userPosition) {
            bool enable = val.toBool();
            b.noSlope->setEnabled(!enable);
            b.y1->setEnabled(enable);
            b.y2->setEnabled(enable);
            }
      else if (ii.w == b.noSlope) {
            bool enable = !val.toBool();
            b.userPosition->setEnabled(enable);
            b.y1->setEnabled(enable);
            b.y2->setEnabled(enable);
            }
      InspectorElementBase::setValue(ii, val);
      }
}

