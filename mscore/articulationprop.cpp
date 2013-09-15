//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: articulation.cpp -1   $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "libmscore/articulation.h"
#include "articulationprop.h"
#include "libmscore/sym.h"
#include "libmscore/score.h"
#include "libmscore/chordrest.h"
#include "libmscore/system.h"
#include "libmscore/measure.h"
#include "libmscore/staff.h"
#include "libmscore/stafftype.h"
#include "libmscore/part.h"
#include "libmscore/segment.h"
#include "libmscore/undo.h"

namespace Ms {

//---------------------------------------------------------
//   ArticulationProperties
//---------------------------------------------------------

ArticulationProperties::ArticulationProperties(Articulation* na, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      articulation = na;

      ChordRest* cr = articulation->chordRest();
      if (cr) {
            Segment* segment       = cr->segment();
            Part* part             = articulation->staff()->part();
            Instrument* instrument = part->instr(segment->tick());

//      const QList<NamedEventList>& midiActions() const;
//      const QList<MidiArticulation>& articulation() const;
//      const QList<Channel>& channel() const;

            foreach(const Channel& a, instrument->channel()) {
                  if (a.name.isEmpty())
                        channelList->addItem(tr("normal"));
                  else
                        channelList->addItem(a.name);
                  }
            foreach(const NamedEventList& el, instrument->midiActions()) {
                  midiActionList->addItem(el.name);
                  }
            }

#if 0
      foreach(const NamedEventList& e, instrument->midiActions)
            midiActionList->addItem(e.name);
      articulationChange->setChecked(!articulation->articulationName().isEmpty());
      midiAction->setChecked(!articulation->midiActionName().isEmpty());

      if (!articulation->articulationName().isEmpty()) {
            QList<QListWidgetItem*> wl = articulationList
               ->findItems(st->articulationName(), Qt::MatchExactly);
            if (!wl.isEmpty())
                  articulationList->setCurrentRow(articulationList->row(wl[0]));
            }
      if (!articulation->midiActionName().isEmpty()) {
            QList<QListWidgetItem*> wl = midiActionList
               ->findItems(st->midiActionName(), Qt::MatchExactly);
            if (!wl.isEmpty())
                  midiActionList->setCurrentRow(midiActionList->row(wl[0]));
            }
#endif

      direction->setCurrentIndex(int(articulation->direction()));
      anchor->setCurrentIndex(int(articulation->anchor()));

      connect(this, SIGNAL(accepted()), SLOT(saveValues()));
      }

//---------------------------------------------------------
//   saveValues
//---------------------------------------------------------

void ArticulationProperties::saveValues()
      {
#if 0
      if (articulationChange->isChecked()) {
            QListWidgetItem* i = articulationList->currentItem();
            if (i)
                  staffText->setChannelName(i->text());
            }
      if (midiAction->isChecked()) {
            QListWidgetItem* i = midiActionList->currentItem();
            if (i)
                  staffText->setMidiActionName(i->text());
            }
#endif
      if (int(articulation->direction()) != direction->currentIndex())
            articulation->score()->undo(new ChangeProperty(articulation,
               P_DIRECTION, direction->currentIndex()));

      if (int(articulation->anchor()) != anchor->currentIndex())
            articulation->score()->undo(new ChangeProperty(articulation,
               P_ARTICULATION_ANCHOR, anchor->currentIndex()));
      }
}

