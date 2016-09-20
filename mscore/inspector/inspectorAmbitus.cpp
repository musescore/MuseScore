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

enum AmbitusControl : char {
      COLOR, VISIBLE, OFF_X, OFF_Y,             // Element controls
      HEADGROUP, HEADTYPE, DIRECTION,           // Range controls
      HASLINE,
      LINEWIDTH,
      TOPTPC, BOTTOMTPC, TOPOCTAVE, BOTTOMOCTAVE,
      LEADINGSPACE                              // Segment controls
      };

//---------------------------------------------------------
//   InspectorAmbitus
//---------------------------------------------------------

InspectorAmbitus::InspectorAmbitus(QWidget* parent)
   : InspectorElementBase(parent)
      {
      r.setupUi(addWidget());
      s.setupUi(addWidget());

      static const NoteHead::Group heads[] = {
            NoteHead::Group::HEAD_NORMAL,
            NoteHead::Group::HEAD_CROSS,
            NoteHead::Group::HEAD_DIAMOND,
            NoteHead::Group::HEAD_TRIANGLE,
            NoteHead::Group::HEAD_SLASH,
            NoteHead::Group::HEAD_XCIRCLE,
            NoteHead::Group::HEAD_DO,
            NoteHead::Group::HEAD_RE,
            NoteHead::Group::HEAD_MI,
            NoteHead::Group::HEAD_FA,
            NoteHead::Group::HEAD_SOL,
            NoteHead::Group::HEAD_LA,
            NoteHead::Group::HEAD_TI,
            NoteHead::Group::HEAD_BREVIS_ALT
            };
      static const Tpc tpcs[] = {
            Tpc::TPC_INVALID,
            Tpc::TPC_C_BB, Tpc::TPC_C_B, Tpc::TPC_C, Tpc::TPC_C_S, Tpc::TPC_C_SS,
            Tpc::TPC_D_BB, Tpc::TPC_D_B, Tpc::TPC_D, Tpc::TPC_D_S, Tpc::TPC_D_SS,
            Tpc::TPC_E_BB, Tpc::TPC_E_B, Tpc::TPC_E, Tpc::TPC_E_S, Tpc::TPC_E_SS,
            Tpc::TPC_F_BB, Tpc::TPC_F_B, Tpc::TPC_F, Tpc::TPC_F_S, Tpc::TPC_F_SS,
            Tpc::TPC_G_BB, Tpc::TPC_G_B, Tpc::TPC_G, Tpc::TPC_G_S, Tpc::TPC_G_SS,
            Tpc::TPC_A_BB, Tpc::TPC_A_B, Tpc::TPC_A, Tpc::TPC_A_S, Tpc::TPC_A_SS,
            Tpc::TPC_B_BB, Tpc::TPC_B_B, Tpc::TPC_B, Tpc::TPC_B_S, Tpc::TPC_B_SS,
      };

      //
      // fix order of noteheads and tpc's
      //
      for (int i = 0; i < int(NoteHead::Group::HEAD_GROUPS); ++i)
            r.noteHeadGroup->setItemData(i, int(heads[i]));
      // noteHeadType starts at -1
      for (int i = 0; i < 5; ++i)
            r.noteHeadType->setItemData(i, i-1);
      // set proper itemdata for TPC combos
      for (int i = 0; i < Tpc::TPC_MAX-Tpc::TPC_MIN+2; ++i) {
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

      const std::vector<InspectorItem> iiList = {
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
            };

      mapSignals(iiList);
      connect(r.updateRange, SIGNAL(clicked()), this, SLOT(updateRange()) );
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------
/*
void InspectorAmbitus::setElement()
      {
      Ambitus* range = toRange(inspector->element());

//      int octave = range->topPitch() / 12;
//      static_cast<QSpinBox*>(iList[AmbitusControl::TOPOCTAVE].w)->setValue(octave);
//      octave = range->bottomPitch() / 12;
//      static_cast<QSpinBox*>(iList[AmbitusControl::BOTTOMOCTAVE].w)->setValue(octave);
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
      if (idx >= AmbitusControl::TOPTPC && idx <= AmbitusControl::BOTTOMOCTAVE) {
            setElement();
            }
      }

}

//---------------------------------------------------------
//   on updateRange clicked
//---------------------------------------------------------

void Ms::InspectorAmbitus::updateRange()
{
      Ambitus* range = toAmbitus(inspector->element());
      range->updateRange();
      range->layout();              // redo layout
      setElement();                 // set Inspector values to range properties
      valueChanged(AmbitusControl::TOPTPC);         // force score to notice new range properties
}

