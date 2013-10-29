//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "libmscore/score.h"
//#include "libmscore/chord.h"
//#include "libmscore/note.h"
//#include "libmscore/notedot.h"
//#include "libmscore/beam.h"
//#include "libmscore/stem.h"
//#include "libmscore/hook.h"
#include "libmscore/rangesymbol.h"
#include "inspector.h"
#include "inspectorRange.h"

namespace Ms {

enum {
      COLOR, VISIBLE, OFF_X, OFF_Y,             // Element controls
      HEADGROUP, HEADTYPE, DIRECTION,           // Range controls
      HASLINE,
      LINEWIDTH,
      TOPTPC, BOTTOMTPC, TOPOCTAVE, BOTTOMOCTAVE,
      LEADINGSPACE, TRAILINGSPACE               // Segment controls
      };

//---------------------------------------------------------
//   InspectorRange
//---------------------------------------------------------

InspectorRange::InspectorRange(QWidget* parent)
   : InspectorBase(parent)
      {
      b.setupUi(addWidget());
      r.setupUi(addWidget());
      s.setupUi(addWidget());

      static const int heads[] = {
            Note::HEAD_NORMAL,
            Note::HEAD_CROSS,
            Note::HEAD_DIAMOND,
            Note::HEAD_TRIANGLE,
            Note::HEAD_SLASH,
            Note::HEAD_XCIRCLE,
            Note::HEAD_DO,
            Note::HEAD_RE,
            Note::HEAD_MI,
            Note::HEAD_FA,
            Note::HEAD_SOL,
            Note::HEAD_LA,
            Note::HEAD_TI,
            Note::HEAD_BREVIS_ALT
            };
      static const int tpcs[] = {
            INVALID_TPC,
            TPC_C_BB, TPC_C_B, TPC_C, TPC_C_S, TPC_C_SS,
            TPC_D_BB, TPC_D_B, TPC_D, TPC_D_S, TPC_D_SS,
            TPC_E_BB, TPC_E_B, TPC_E, TPC_E_S, TPC_E_SS,
            TPC_F_BB, TPC_F_B, TPC_F, TPC_F_S, TPC_F_SS,
            TPC_G_BB, TPC_G_B, TPC_G, TPC_G_S, TPC_G_SS,
            TPC_A_BB, TPC_A_B, TPC_A, TPC_A_S, TPC_A_SS,
            TPC_B_BB, TPC_B_B, TPC_B, TPC_B_S, TPC_B_SS,
      };

      //
      // fix order of note heads and tpc's
      //
      for (int i = 0; i < Note::HEAD_GROUPS; ++i)
            r.noteHeadGroup->setItemData(i, heads[i]);
      // noteHeadType starts at -1
      for (int i = 0; i < 5; ++i)
            r.noteHeadType->setItemData(i, i-1);
      for (int i = 0; i < TPC_MAX-TPC_MIN+2; ++i) {
            r.topTpc->   setItemData(i, tpcs[i]);
            r.bottomTpc->setItemData(i, tpcs[i]);
            }

      iList = {
            { P_COLOR,          0, 0, b.color,         b.resetColor         },
            { P_VISIBLE,        0, 0, b.visible,       b.resetVisible       },
            { P_USER_OFF,       0, 0, b.offsetX,       b.resetX             },
            { P_USER_OFF,       1, 0, b.offsetY,       b.resetY             },

            { P_HEAD_GROUP,     0, 0, r.noteHeadGroup, r.resetNoteHeadGroup },
            { P_HEAD_TYPE,      0, 0, r.noteHeadType,  r.resetNoteHeadType  },
            { P_MIRROR_HEAD,    0, 0, r.direction,     r.resetDirection     },
            { P_GHOST,          0, 0, r.hasLine,       r.resetHasLine       },      // recycled property
            { P_LINE_WIDTH,     0, 0, r.lineWidth,     r.resetLineWidth     },
            { P_TPC,            0, 0, r.topTpc,        nullptr              },
            { P_FBPARENTHESIS1, 0, 0, r.bottomTpc,     nullptr              },      // recycled property
            { P_FBPARENTHESIS3, 0, 0, r.topOctave,     nullptr              },      // recycled property
            { P_FBPARENTHESIS4, 0, 0, r.bottomOctave,  nullptr              },      // recycled property

            { P_LEADING_SPACE,  0, 1, s.leadingSpace,  s.resetLeadingSpace  },
            { P_TRAILING_SPACE, 0, 1, s.trailingSpace, s.resetTrailingSpace }
            };

      mapSignals();

      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------
/*
void InspectorRange::setElement()
      {
      Range* range = static_cast<Range*>(inspector->element());

//      int octave = range->topPitch() / 12;
//      static_cast<QSpinBox*>(iList[TOPOCTAVE].w)->setValue(octave);
//      octave = range->bottomPitch() / 12;
//      static_cast<QSpinBox*>(iList[BOTTOMOCTAVE].w)->setValue(octave);
//      InspectorBase::setElement();
      }
*/
}

