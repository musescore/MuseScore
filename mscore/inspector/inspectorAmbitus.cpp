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
      static const Tpc tpcs[] = {
            INVALID,
            C_BB, C_B, C, C_S, C_SS,
            D_BB, D_B, D, D_S, D_SS,
            E_BB, E_B, E, E_S, E_SS,
            F_BB, F_B, F, F_S, F_SS,
            G_BB, G_B, G, G_S, G_SS,
            A_BB, A_B, A, A_S, A_SS,
            B_BB, B_B, B, B_S, B_SS,
      };

      //
      // fix order of note heads and tpc's
      //
      for (int i = 0; i < int(NoteHeadGroup::HEAD_GROUPS); ++i)
            r.noteHeadGroup->setItemData(i, int(heads[i]));
      // noteHeadType starts at -1
      for (int i = 0; i < 5; ++i)
            r.noteHeadType->setItemData(i, i-1);
      // set proper itemdata for TPC combos
      for (int i = 0; i < Tpc::MAX-Tpc::MIN+2; ++i) {
            r.topTpc->   setItemData(i, tpcs[i]);
            r.bottomTpc->setItemData(i, tpcs[i]);
            }
      // make first item of each TPC combo ("[undefined]") unselectable
      const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(r.topTpc->model());
      QStandardItem* item = model->item(0);
      item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
      model = qobject_cast<const QStandardItemModel*>(r.bottomTpc->model());
      item = model->item(0);
      item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));

      iList = {
            { P_ID::COLOR,          0, 0, b.color,         b.resetColor         },
            { P_ID::VISIBLE,        0, 0, b.visible,       b.resetVisible       },
            { P_ID::USER_OFF,       0, 0, b.offsetX,       b.resetX             },
            { P_ID::USER_OFF,       1, 0, b.offsetY,       b.resetY             },

            { P_ID::HEAD_GROUP,     0, 0, r.noteHeadGroup, r.resetNoteHeadGroup },
            { P_ID::HEAD_TYPE,      0, 0, r.noteHeadType,  r.resetNoteHeadType  },
            { P_ID::MIRROR_HEAD,    0, 0, r.direction,     r.resetDirection     },
            { P_ID::GHOST,          0, 0, r.hasLine,       r.resetHasLine       },      // recycled property
            { P_ID::LINE_WIDTH,     0, 0, r.lineWidth,     r.resetLineWidth     },
            { P_ID::TPC1,           0, 0, r.topTpc,        nullptr              },
            { P_ID::FBPARENTHESIS1, 0, 0, r.bottomTpc,     nullptr              },      // recycled property
            { P_ID::FBPARENTHESIS3, 0, 0, r.topOctave,     nullptr              },      // recycled property
            { P_ID::FBPARENTHESIS4, 0, 0, r.bottomOctave,  nullptr              },      // recycled property

            { P_ID::LEADING_SPACE,  0, 1, s.leadingSpace,  s.resetLeadingSpace  },
            { P_ID::TRAILING_SPACE, 0, 1, s.trailingSpace, s.resetTrailingSpace }
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
//   on updateRange clicked
//---------------------------------------------------------

void Ms::InspectorAmbitus::updateRange()
{
      Ambitus* range = static_cast<Ambitus*>(inspector->element());
      range->updateRange();
      range->layout();              // redo layout
      setElement();                 // set Inspector values to range properties
      valueChanged(TOPTPC);         // force score to notice new range properties
}

