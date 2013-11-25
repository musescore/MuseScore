//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2013 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspector.h"
#include "inspectorAmbitus.h"
#include "libmscore/ambitus.h"
#include "libmscore/score.h"

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
//   InspectorAmbitus
//---------------------------------------------------------

InspectorAmbitus::InspectorAmbitus(QWidget* parent)
   : InspectorBase(parent)
      {
      b.setupUi(addWidget());
      r.setupUi(addWidget());
      s.setupUi(addWidget());

      static const NoteHeadGroup heads[] = {
            NoteHeadGroup::HEAD_NORMAL,
            NoteHeadGroup::HEAD_CROSS,
            NoteHeadGroup::HEAD_DIAMOND,
            NoteHeadGroup::HEAD_TRIANGLE,
            NoteHeadGroup::HEAD_SLASH,
            NoteHeadGroup::HEAD_XCIRCLE,
            NoteHeadGroup::HEAD_DO,
            NoteHeadGroup::HEAD_RE,
            NoteHeadGroup::HEAD_MI,
            NoteHeadGroup::HEAD_FA,
            NoteHeadGroup::HEAD_SOL,
            NoteHeadGroup::HEAD_LA,
            NoteHeadGroup::HEAD_TI,
            NoteHeadGroup::HEAD_BREVIS_ALT
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
      for (int i = 0; i < int(NoteHeadGroup::HEAD_GROUPS); ++i)
            r.noteHeadGroup->setItemData(i, int(heads[i]));
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
      connect(r.updateRange, SIGNAL(clicked()), this, SLOT(updateRange()) );

      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------
/*
void InspectorAmbitus::setElement()
      {
      Ambitus* range = static_cast<Range*>(inspector->element());

//      int octave = range->topPitch() / 12;
//      static_cast<QSpinBox*>(iList[TOPOCTAVE].w)->setValue(octave);
//      octave = range->bottomPitch() / 12;
//      static_cast<QSpinBox*>(iList[BOTTOMOCTAVE].w)->setValue(octave);
//      InspectorBase::setElement();
      }
*/
//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorAmbitus::valueChanged(int idx)
      {
      InspectorBase::valueChanged(idx);
      // if either tpc or octave is changed, notes can have been swapped
      // (to keep top above bottom): reload data
      if (idx >= TOPTPC && idx <= BOTTOMOCTAVE) {
            setElement();
            }
      }

}

//---------------------------------------------------------
//   on updateRage clicked
//---------------------------------------------------------

void Ms::InspectorAmbitus::updateRange()
{
      Ambitus* range = static_cast<Ambitus*>(inspector->element());
      range->updateRange();
      range->layout();              // redo layout
      setElement();                 // set Inspector values to range properties
      valueChanged(TOPTPC);         // force score to notice new range properties
}

