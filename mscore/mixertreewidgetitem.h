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

#ifndef __MIXERTREEWIDGETITEM__
#define __MIXERTREEWIDGETITEM__

namespace Ms {

class MixerTrackChannel;
class MixerTrackItem;
class Part;
class Instrument;
class Channel;
class Score;
/*
 MixerTreeWidgetItem
 
 subclass of QTreeWidget that:
 - will construct a MixerTreeWidgetItem from a Part (including any children)
 - host a MixerTrackItem for processing user changes and updating controls
   when changes are signalled from outwith the mixer
*/
class MixerTrackChannel;
      
class MixerTreeWidgetItem : public QTreeWidgetItem
      {
      MixerTrackItem* _mixerTrackItem;
      MixerTrackChannel* _mixerTrackChannel;

   public:
      MixerTreeWidgetItem(Part* part, Score* score, QTreeWidget* parent);
      MixerTreeWidgetItem(Channel* channel, Instrument* instrument, Part* part);
      ~MixerTreeWidgetItem();

      MixerTrackItem* mixerTrackItem() { return _mixerTrackItem; };
      MixerTrackChannel* mixerTrackChannel();
      };


/*
NonEditableItemDelegate

Allow some columns to be non-editable in the tree view by means of a delegate.

Usage
treeWidget->setItemDelegateForColumn(column, new  NonEditableItemDelegate(treeWidget));
This will turn editing OFF for the selected column when editing is ON for the item.
*/

class NonEditableItemDelegate : public QStyledItemDelegate
      {
public:
      NonEditableItemDelegate(QObject* parent = nullptr);
      virtual QWidget* createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const;

      };

}
#endif // __MIXERTREEWIDGETITEM__
