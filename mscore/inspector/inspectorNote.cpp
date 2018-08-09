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
#include "libmscore/staff.h"
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
            NoteHead::Group::HEAD_PLUS,
            NoteHead::Group::HEAD_XCIRCLE,
            NoteHead::Group::HEAD_WITHX,
            NoteHead::Group::HEAD_TRIANGLE_UP,
            NoteHead::Group::HEAD_TRIANGLE_DOWN,
            NoteHead::Group::HEAD_SLASHED1,
            NoteHead::Group::HEAD_SLASHED2,
            NoteHead::Group::HEAD_DIAMOND,
            NoteHead::Group::HEAD_DIAMOND_OLD,
            NoteHead::Group::HEAD_CIRCLED,
            NoteHead::Group::HEAD_CIRCLED_LARGE,
            NoteHead::Group::HEAD_LARGE_ARROW,

            NoteHead::Group::HEAD_SLASH,
            NoteHead::Group::HEAD_BREVIS_ALT,

            NoteHead::Group::HEAD_DO,
            NoteHead::Group::HEAD_RE,
            NoteHead::Group::HEAD_MI,
            NoteHead::Group::HEAD_FA,
            NoteHead::Group::HEAD_SOL,
            NoteHead::Group::HEAD_LA,
            NoteHead::Group::HEAD_TI
            };

      //
      // fix order of noteheads
      //
      for (auto head : heads)
            n.noteHeadGroup->addItem(NoteHead::group2userName(head), int(head));

      // noteHeadType starts at -1: correct values and count one item more (HEAD_AUTO)
      for (int i = 0; i <= int(NoteHead::Type::HEAD_TYPES); ++i) {
            n.noteHeadType->addItem(NoteHead::type2userName(NoteHead::Type(i - 1)));
            n.noteHeadType->setItemData(i, i - 1);
            }

      const std::vector<InspectorItem> iiList = {
            { Pid::SMALL,          0, n.small,         n.resetSmall         },
            { Pid::HEAD_GROUP,     0, n.noteHeadGroup, n.resetNoteHeadGroup },
            { Pid::HEAD_TYPE,      0, n.noteHeadType,  n.resetNoteHeadType  },
            { Pid::MIRROR_HEAD,    0, n.mirrorHead,    n.resetMirrorHead    },
            { Pid::PLAY,           0, n.play,          n.resetPlay          },
            { Pid::TUNING,         0, n.tuning,        n.resetTuning        },
            { Pid::VELO_TYPE,      0, n.velocityType,  n.resetVelocityType  },
            { Pid::VELO_OFFSET,    0, n.velocity,      n.resetVelocity      },
            { Pid::FIXED,          0, n.fixed,         n.resetFixed         },
            { Pid::FIXED_LINE,     0, n.fixedLine,     n.resetFixedLine     },

            { Pid::USER_OFF,       1, c.offset,        c.resetOffset        },
            { Pid::SMALL,          1, c.small,         c.resetSmall         },
            { Pid::NO_STEM,        1, c.stemless,      c.resetStemless      },
            { Pid::STEM_DIRECTION, 1, c.stemDirection, c.resetStemDirection },

            { Pid::LEADING_SPACE,  2, s.leadingSpace,  s.resetLeadingSpace  },
            };
      const std::vector<InspectorPanel> ppList = {
            { s.title, s.panel },
            { c.title, c.panel },
            { n.title, n.panel },
            };
      mapSignals(iiList, ppList);

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

      bool isNHGroupEnabled = note->chord()->staff()->isPitchedStaff(note->tick())
         && note->chord()->staff()->staffType(note->tick())->noteHeadScheme() == NoteHeadScheme::HEAD_NORMAL;
      n.noteHeadGroup->setEnabled(isNHGroupEnabled);

      InspectorElementBase::setElement();

      //must be placed after InspectorBase::setElement() cause the last one sets resetButton enability
      if (!isNHGroupEnabled)
            n.resetNoteHeadGroup->setEnabled(false);

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
            dot->score()->update();
            inspector->update();
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
            dot->score()->update();
            inspector->update();
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
            dot->score()->update();
            inspector->update();
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
            dot->score()->update();
            inspector->update();
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
            note->score()->update();
            inspector->update();
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
            note->score()->update();
            inspector->update();
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
            note->score()->update();
            inspector->update();
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
            note->score()->update();
            inspector->update();
            }
      }

}

