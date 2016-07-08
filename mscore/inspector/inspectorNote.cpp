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
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/notedot.h"
#include "libmscore/beam.h"
#include "libmscore/stem.h"
#include "libmscore/hook.h"
#include "libmscore/tuplet.h"
#include "inspector.h"
#include "inspectorNote.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorNote
//---------------------------------------------------------

InspectorNote::InspectorNote(QWidget* parent)
   : InspectorElementBase(parent)
      {
      s.setupUi(addWidget());
      c.setupUi(addWidget());
      n.setupUi(addWidget());

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

      //
      // fix order of noteheads
      //
      for (unsigned i = 0; i < sizeof(heads)/sizeof(*heads); ++i) {
            n.noteHeadGroup->addItem(qApp->translate("noteheadnames", NoteHead::groupToGroupName(heads[i])));
            n.noteHeadGroup->setItemData(i, QVariant(int(heads[i])));
            }

      // noteHeadType starts at -1: correct values and count one item more (HEAD_AUTO)
      for (int i = 0; i <= int(NoteHead::Type::HEAD_TYPES); ++i)
            n.noteHeadType->setItemData(i, i-1);

      const std::vector<InspectorItem> iiList = {
            { P_ID::SMALL,          0, 0, n.small,         n.resetSmall         },
            { P_ID::HEAD_GROUP,     0, 0, n.noteHeadGroup, n.resetNoteHeadGroup },
            { P_ID::HEAD_TYPE,      0, 0, n.noteHeadType,  n.resetNoteHeadType  },
            { P_ID::MIRROR_HEAD,    0, 0, n.mirrorHead,    n.resetMirrorHead    },
            { P_ID::DOT_POSITION,   0, 0, n.dotPosition,   n.resetDotPosition   },
            { P_ID::PLAY,           0, 0, n.play,          n.resetPlay          },
            { P_ID::TUNING,         0, 0, n.tuning,        n.resetTuning        },
            { P_ID::VELO_TYPE,      0, 0, n.velocityType,  n.resetVelocityType  },
            { P_ID::VELO_OFFSET,    0, 0, n.velocity,      n.resetVelocity      },
            { P_ID::FIXED,          0, 0, n.fixed,         n.resetFixed         },
            { P_ID::FIXED_LINE,     0, 0, n.fixedLine,     n.resetFixedLine     },

            { P_ID::USER_OFF,       0, 1, c.offsetX,       c.resetX             },
            { P_ID::USER_OFF,       1, 1, c.offsetY,       c.resetY             },
            { P_ID::SMALL,          0, 1, c.small,         c.resetSmall         },
            { P_ID::NO_STEM,        0, 1, c.stemless,      c.resetStemless      },
            { P_ID::STEM_DIRECTION, 0, 1, c.stemDirection, c.resetStemDirection },

            { P_ID::LEADING_SPACE,  0, 2, s.leadingSpace,  s.resetLeadingSpace  },
            };
      mapSignals(iiList);

      connect(n.dot1,     SIGNAL(clicked()),     SLOT(dot1Clicked()));
      connect(n.dot2,     SIGNAL(clicked()),     SLOT(dot2Clicked()));
      connect(n.dot3,     SIGNAL(clicked()),     SLOT(dot3Clicked()));
      connect(n.dot4,     SIGNAL(clicked()),     SLOT(dot4Clicked()));
      connect(n.hook,     SIGNAL(clicked()),     SLOT(hookClicked()));
      connect(n.stem,     SIGNAL(clicked()),     SLOT(stemClicked()));
      connect(n.beam,     SIGNAL(clicked()),     SLOT(beamClicked()));
      connect(n.tuplet,   SIGNAL(clicked()),     SLOT(tupletClicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorNote::setElement()
      {
      Note* note = toNote(inspector->element());

      int i = note->dots().size();
      n.dot1->setEnabled(i > 0);
      n.dot2->setEnabled(i > 1);
      n.dot3->setEnabled(i > 2);
      n.dot4->setEnabled(i > 3);
      n.stem->setEnabled(note->chord()->stem());
      n.hook->setEnabled(note->chord()->hook());
      n.beam->setEnabled(note->chord()->beam());
      n.tuplet->setEnabled(note->chord()->tuplet());
      InspectorElementBase::setElement();
      bool nograce = !note->chord()->isGrace();
      s.leadingSpace->setEnabled(nograce);
      s.resetLeadingSpace->setEnabled(nograce && s.leadingSpace->value());
      }

//---------------------------------------------------------
//   dot1Clicked
//---------------------------------------------------------

void InspectorNote::dot1Clicked()
      {
      Note* note = toNote(inspector->element());
      if (note == 0)
            return;
      if (note->dots().size() > 0) {
            NoteDot* dot = note->dot(0);
            dot->score()->select(dot);
            inspector->setElement(dot);
            dot->score()->update();
            }
      }

//---------------------------------------------------------
//   dot2Clicked
//---------------------------------------------------------

void InspectorNote::dot2Clicked()
      {
      Note* note = toNote(inspector->element());
      if (note == 0)
            return;
      if (note->dots().size() > 1) {
            NoteDot* dot = note->dot(1);
            dot->score()->select(dot);
            inspector->setElement(dot);
            dot->score()->update();
            }
      }

//---------------------------------------------------------
//   dot3Clicked
//---------------------------------------------------------

void InspectorNote::dot3Clicked()
      {
      Note* note = toNote(inspector->element());
      if (note == 0)
            return;
      if (note->dots().size() > 2) {
            NoteDot* dot = note->dot(2);
            dot->score()->select(dot);
            inspector->setElement(dot);
            dot->score()->update();
            }
      }

//---------------------------------------------------------
//   dot4Clicked
//---------------------------------------------------------

void InspectorNote::dot4Clicked()
      {
      Note* note = toNote(inspector->element());
      if (note == 0)
            return;
      if (note->dots().size() > 3) {
            NoteDot* dot = note->dot(3);
            dot->score()->select(dot);
            inspector->setElement(dot);
            dot->score()->update();
            }
      }

//---------------------------------------------------------
//   hookClicked
//---------------------------------------------------------

void InspectorNote::hookClicked()
      {
      Note* note = toNote(inspector->element());
      if (note == 0)
            return;
      Hook* hook = note->chord()->hook();
      if (hook) {
            note->score()->select(hook);
            inspector->setElement(hook);
            note->score()->update();
            }
      }

//---------------------------------------------------------
//   stemClicked
//---------------------------------------------------------

void InspectorNote::stemClicked()
      {
      Note* note = toNote(inspector->element());
      if (note == 0)
            return;
      Stem* stem = note->chord()->stem();
      if (stem) {
            note->score()->select(stem);
            inspector->setElement(stem);
            note->score()->update();
            }
      }

//---------------------------------------------------------
//   beamClicked
//---------------------------------------------------------

void InspectorNote::beamClicked()
      {
      Note* note = toNote(inspector->element());
      if (note == 0)
            return;
      Beam* beam = note->chord()->beam();
      if (beam) {
            note->score()->select(beam);
            inspector->setElement(beam);
            note->score()->update();
            }
      }

//---------------------------------------------------------
//   tupletClicked
//---------------------------------------------------------

void InspectorNote::tupletClicked()
      {
      Note* note = toNote(inspector->element());
      if (note == 0)
            return;
      Tuplet* tuplet = note->chord()->tuplet();
      if (tuplet) {
            note->score()->select(tuplet);
            inspector->setElement(tuplet);
            note->score()->update();
            }
      }

}

