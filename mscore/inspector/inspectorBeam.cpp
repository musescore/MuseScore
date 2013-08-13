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
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      b.setupUi(addWidget());

      iList = {
            { P_COLOR,          0, false, e.color,        e.resetColor        },
            { P_VISIBLE,        0, false, e.visible,      e.resetVisible      },
            { P_USER_OFF,       0, false, e.offsetX,      e.resetX            },
            { P_USER_OFF,       1, false, e.offsetY,      e.resetY            },
            { P_STEM_DIRECTION, 0, false, b.direction,    b.resetDirection    },
            { P_DISTRIBUTE,     0, false, b.distribute,   b.resetDistribute   },
            { P_GROW_LEFT,      0, false, b.growLeft,     b.resetGrowLeft     },
            { P_GROW_RIGHT,     0, false, b.growRight,    b.resetGrowRight    },
            { P_BEAM_NO_SLOPE,  0, false, b.noSlope,      b.resetNoSlope      },
            { P_USER_MODIFIED,  0, false, b.userPosition, b.resetUserPosition },
            { P_BEAM_POS,       0, false, b.y1,           0                   },
            { P_BEAM_POS,       1, false, b.y2,           0                   }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorBeam::valueChanged(int idx)
      {
      if (iList[idx].t == P_USER_MODIFIED) {
            bool val = getValue(iList[idx]).toBool();
            iList[8].w->setEnabled(!val);
            iList[10].w->setEnabled(val);
            iList[11].w->setEnabled(val);
            }
      InspectorBase::valueChanged(idx);
      }

void InspectorBeam::setValue(const InspectorItem& ii, const QVariant& val)
      {
      if (ii.w == b.userPosition) {
            bool enable = val.toBool();
            iList[8].w->setEnabled(!enable);
            iList[10].w->setEnabled(enable);
            iList[11].w->setEnabled(enable);
            }
      else if (ii.w == b.noSlope) {
            bool enable = !val.toBool();
            iList[9].w->setEnabled(enable);
            iList[10].w->setEnabled(enable);
            iList[11].w->setEnabled(enable);
            }
      InspectorBase::setValue(ii, val);
      }
}

