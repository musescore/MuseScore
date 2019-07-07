//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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

#include "mixertreewidgetitem.h"

#include "musescore.h"                    // required for access to synti
#include "synthesizer/msynthesizer.h"     // required for MidiPatch
#include "seq.h"

#include "libmscore/score.h"
#include "libmscore/part.h"

#include "mixertrackchannel.h"


namespace Ms {

// MixerTreeWidgetItem class

      //TODO: can I initialise this with the mixerTrackItem itself?
MixerTreeWidgetItem::MixerTreeWidgetItem(Part* part, Score* score, QTreeWidget* treeWidget)
      {
      _mixerTrackItem = new MixerTrackItem(part, score);
      _mixerTrackChannel = new MixerTrackChannel(this);

      setText(0, part->partName());
      setToolTip(0, part->partName());

      // make the row editable - but the tree itself will only allow editing in column 0

      // note: use of QFlag constructor is required to avoid errrors when compiler is strict
      Qt::ItemFlags itemFlags = QFlag(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      setFlags(itemFlags);


      // check for secondary channels and add MixerTreeWidgetItem children if required
      const InstrumentList* partInstrumentList = part->instruments(); //Add per channel tracks
      
      // partInstrumentList is of type: map<const int, Instrument*>
      for (auto partInstrumentListItem = partInstrumentList->begin(); partInstrumentListItem != partInstrumentList->end(); ++partInstrumentListItem) {
            
            Instrument* instrument = partInstrumentListItem->second;
            if (instrument->channel().size() <= 1)
                  continue;
            
            for (int i = 0; i < instrument->channel().size(); ++i) {
                  Channel* channel = instrument->playbackChannel(i, score->masterScore());
                  MixerTreeWidgetItem* child = new MixerTreeWidgetItem(channel, instrument, part);

                  addChild(child);
                  treeWidget->setItemWidget(child, 1, child->mixerTrackChannel());
                  // make the row non-editable - sub-instruments can't change their name
                  child->setFlags(Qt::ItemIsEnabled);
                  }
            }
      }

MixerTreeWidgetItem::MixerTreeWidgetItem(Channel* channel, Instrument* instrument, Part* part)
      {
      setText(0, channel->name());
      setToolTip(0, QString("%1 - %2").arg(part->partName()).arg(channel->name()));
      _mixerTrackItem = new MixerTrackItem(MixerTrackItem::TrackType::CHANNEL, part, instrument, channel);
      _mixerTrackChannel = new MixerTrackChannel(this);
      }


MixerTrackChannel* MixerTreeWidgetItem::mixerTrackChannel() {
      return _mixerTrackChannel;
      }

MixerTreeWidgetItem:: ~MixerTreeWidgetItem()
      {
      _mixerTrackChannel->setNotifier(nullptr);      // ensure it stops listening
      delete _mixerTrackItem;
      // note: the _MixerTrackChannel is taken care of (owned) after the setItemWidget call
      // and so does not need to (and must not) be deleted here
      }


NonEditableItemDelegate::NonEditableItemDelegate(QObject* parent) : QStyledItemDelegate(parent)
      {
      }

QWidget* NonEditableItemDelegate::createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const
      {
      return nullptr;
      }


}

