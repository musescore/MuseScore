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

#include "mixertreewidget.h"
#include "mixertrackitem.h"
#include "mixertrackchannel.h"
#include "mixer.h"
#include "mixeroptions.h"
#include "libmscore/part.h"

#include <QTreeWidget>
#include <QMenu>

#define MIXERTREE_INVALID_INDEX -1

namespace Ms {

MixerTreeWidget::MixerTreeWidget(QWidget *parent) :
      QTreeWidget(parent), savedSelectionTopLevelIndex (MIXERTREE_INVALID_INDEX), savedSelectionChildIndex(MIXERTREE_INVALID_INDEX), masterChannelTreeWidget(nullptr)
      {
      setAlternatingRowColors(true);
      setColumnCount(2);
      setHeaderLabels({tr("Instrument"), tr("Volume")});
      // make all bar column 0, non-editable
      setItemDelegateForColumn(1, new NonEditableItemDelegate (this));

      header()->setSectionResizeMode(0, QHeaderView::Fixed);
      header()->setSectionResizeMode(1, QHeaderView::Fixed);

      setSelectionBehavior(QAbstractItemView::SelectRows);
      adjustHeaderWidths();

      setupContextMenuActions();

      setContextMenuPolicy(Qt::CustomContextMenu);
      setupSlotsAndSignals();

      }

void MixerTreeWidget::setupSlotsAndSignals()
{
      connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), SLOT(selectedItemChanged()));

      connect(header(), SIGNAL(geometriesChanged()), SLOT(adjustHeaderWidths()));
      connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)), SLOT(itemCollapsedOrExpanded(QTreeWidgetItem*)));
      connect(this, SIGNAL(itemCollapsed(QTreeWidgetItem*)), SLOT(itemCollapsedOrExpanded(QTreeWidgetItem*)));
      connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), SLOT(itemChanged(QTreeWidgetItem*, int)));
      connect(this, SIGNAL(customContextMenuRequested(QPoint)), SLOT(contextMenu(QPoint)));

      connect(expandAll, SIGNAL(triggered()), this, SLOT(doExpandAll()));
      connect(collapseAll, SIGNAL(triggered()), this, SLOT(doCollapseAll()));
      connect(resetAll, SIGNAL(triggered()), this, SLOT(doResetAll()));
      connect(resetAllAndZeroVolume, SIGNAL(triggered()), this, SLOT(doResetAllAndZeroVolume()));
}


void MixerTreeWidget::doExpandAll()
      {
      QTreeWidget::expandAll();
      }

void MixerTreeWidget::doCollapseAll()
      {
      QTreeWidget::collapseAll();
      }

void MixerTreeWidget::doResetAll()
      {
      resetAllItemsToDefaultAndSetVolume(MixerTrackItem::defaultVolume);
      }

void MixerTreeWidget::doResetAllAndZeroVolume()
      {
      resetAllItemsToDefaultAndSetVolume(0);
      }

void MixerTreeWidget::setupContextMenuActions()
      {
      expandAll = new QAction(tr("Expand all"));
      collapseAll = new QAction(tr("Collapse all"));
      resetAll = new QAction(tr("Reset Levels (Default Volume)"));
      resetAllAndZeroVolume = new QAction(tr("Reset Levels (Zero Volume)"));
      }

void MixerTreeWidget::contextMenu(QPoint position)
      {
      QMenu* menu = new QMenu();

      expandAll->setEnabled(anyToExpand());
      collapseAll->setEnabled(anyToCollapse());

      resetAllAndZeroVolume->setEnabled(!areAllItemsResetWithVolume(0));
      resetAll->setEnabled(!areAllItemsResetWithVolume(MixerTrackItem::defaultVolume));

      menu->addAction(expandAll);
      menu->addAction(collapseAll);
      menu->addAction(resetAll);
      menu->addAction(resetAllAndZeroVolume);

      menu->exec(this->mapToGlobal(position));
      }


void MixerTreeWidget::setScore(Score* score)
      {
      clear();

      if (score)
            populateTree(score);

      if (savedSelectionTopLevelIndex == MIXERTREE_INVALID_INDEX && topLevelItemCount() > 0) {
            setCurrentItem(this->itemAt(0,0));
            }

     restoreTreeSelection();
      }



void MixerTreeWidget::populateTree(Score* score)
      {
      for (Part* localPart : score->parts()) {
            Part* part = localPart->masterPart();
            // When it's created the item will also create any children and setup their widgets
            MixerTreeWidgetItem* item = new MixerTreeWidgetItem(part, score, this);
            addTopLevelItem(item);
            setItemWidget(item, 1, item->mixerTrackChannel());
            item->setExpanded(part->isExpanded());
            }
      }


// - listen for changes to current item so that the details view can be updated
// also called directly by updateTracks (while signals are disabled)
void MixerTreeWidget::selectedItemChanged()
      {
      if (topLevelItemCount() == 0 || !currentItem()) {
            emit selectedTrackChanged(nullptr);
            return;
            }

      MixerTreeWidgetItem* item = static_cast<MixerTreeWidgetItem*>(currentItem());

      emit selectedTrackChanged(item->mixerTrackItem());
      }



void MixerTreeWidget::updateHeaders() {

      if (!Mixer::getOptions()->secondaryModeOn()) {
            setHeaderLabels({tr("Instrument"), tr("Volume")});
            return;
            }

      QString secondary;

      switch (Mixer::getOptions()->secondarySlider()) {
            case MixerOptions::MixerSecondarySlider::Pan:
                  secondary = tr("Pan");
                  break;
            case MixerOptions::MixerSecondarySlider::Reverb:
                  secondary = tr("Reverb");
                  break;
            case MixerOptions::MixerSecondarySlider::Chorus:
                  secondary = tr("Chorus");
            }

      setHeaderLabels({tr("Instrument"), secondary});
      }



void MixerTreeWidget::setSecondaryMode(bool secondaryMode)
      {
      updateSliders();
      updateHeaders();
      
      
      
      
      }

void MixerTreeWidget::updateSliders()
      {
      // secondary slider style and track colors in the main mixer
      for (int topLevelIndex = 0; topLevelIndex < topLevelItemCount(); topLevelIndex++) {
            QTreeWidgetItem* topLevelItem = this->topLevelItem(topLevelIndex);
            MixerTrackChannel* itemWidget = static_cast<MixerTrackChannel*>(this->itemWidget(topLevelItem, 1));
            itemWidget->updateUiControls();

            for (int childIndex = 0; childIndex < topLevelItem->childCount(); childIndex++) {
                  QTreeWidgetItem* childItem = topLevelItem->child(childIndex);
                  MixerTrackChannel* itemWidget = static_cast<MixerTrackChannel*>(this->itemWidget(childItem, 1));
                  itemWidget->updateUiControls();
                  }
            }
      }


void MixerTreeWidget::adjustHeaderWidths()
      {
      int width = this->width();

      int firstColumnMaximumWidth = 300;
      double ratio = 0.4; // instrument names take up 2/5 and controls 3/5
      int margin = 2;   // factor to avoid triggering horizontal scrolling

      int column0 = int(double(width) * ratio);
      int column1 = int(double(width) * (1-ratio) - margin);

      if (column0 > firstColumnMaximumWidth) {
            column0 = firstColumnMaximumWidth;
            column1 = (width - firstColumnMaximumWidth) - margin;
            }

      header()->resizeSection(0, column0);
      header()->resizeSection(1, column1);

      if (masterChannelTreeWidget) {
            masterChannelTreeWidget->header()->resizeSection(0, column0);
            masterChannelTreeWidget->header()->resizeSection(1, column1);
            }
      }

void MixerTreeWidget::setMasterChannelTreeWidget(QTreeWidget* masterChannelTreeWidget)
      {
      this->masterChannelTreeWidget = masterChannelTreeWidget;
      }

//MARK:- tree changes
//called when an item is edited
void MixerTreeWidget::itemChanged(QTreeWidgetItem* treeWidgetItem, int column)
      {
      MixerTreeWidgetItem* item = static_cast<MixerTreeWidgetItem*>(treeWidgetItem);
      saveTreeSelection();
      item->mixerTrackItem()->setName(item->text(column));
      }



void MixerTreeWidget::resetAllItemsToDefaultAndSetVolume(int volume)
      {

      
      for (int itemIndex = 0; itemIndex < topLevelItemCount(); itemIndex++) {
            MixerTreeWidgetItem* item = static_cast<MixerTreeWidgetItem*>(topLevelItem(itemIndex));
            item->mixerTrackItem()->resetWithVolume(volume);
            }
      }

bool MixerTreeWidget::areAllItemsResetWithVolume(int volume) {

      for (int itemIndex = 0; itemIndex < topLevelItemCount(); itemIndex++) {
            MixerTreeWidgetItem* item = static_cast<MixerTreeWidgetItem*>(topLevelItem(itemIndex));
            if (!item->mixerTrackItem()->isResetWithVolume(volume))
                  return false;
            for (int itemIndex = 0; itemIndex < item->childCount(); itemIndex++) {
                  MixerTreeWidgetItem* childItem = static_cast<MixerTreeWidgetItem*>(item->child(itemIndex));
                  if (!childItem->mixerTrackItem()->isResetWithVolume(volume))
                        return false;
                  }
            }

      return true;
}

      
bool MixerTreeWidget::anyToExpand()
      {
      // if any item has children and is not expanded
      for (int itemIndex = 0; itemIndex < topLevelItemCount(); itemIndex++) {
            QTreeWidgetItem* item = topLevelItem(itemIndex);
            if (item->childCount() > 0 && !item->isExpanded())
                  return true;
            }
      return false;
      }
      
      
bool MixerTreeWidget::anyToCollapse()
      {
      // if any items have children and are expanded
      for (int itemIndex = 0; itemIndex < topLevelItemCount(); itemIndex++) {
            QTreeWidgetItem* item = topLevelItem(itemIndex);
            if (item->childCount() > 0 && item->isExpanded())
                  return true;
            }
      return false;
      }


void MixerTreeWidget::itemCollapsedOrExpanded(QTreeWidgetItem* item) {

      MixerTreeWidgetItem* mixerTreeWidgetItem = static_cast<MixerTreeWidgetItem*>(item);
      MixerTrackItem* mixerTrackItem = mixerTreeWidgetItem->mixerTrackItem();
      if (mixerTrackItem && mixerTrackItem->isPart()) {
            mixerTrackItem->part()->setExpanded(item->isExpanded());
            }
      }

// Used to save the item currently selected in the tree when performing operations
// such as changing the patch. The way changing patches is implemented is that it
// triggers a new setScore() method on the mixer which, in turn, and of necessity,
// forces the channel strips to be built again from scratch. Not clear patch changes
// have to do this, but, currently, they do. This works around that.



void MixerTreeWidget::restoreTreeSelection()
      {
      int topLevel = savedSelectionTopLevelIndex;
      savedSelectionTopLevelIndex = MIXERTREE_INVALID_INDEX;   // indicates no selection currently saved

      // there are no items, so we can't select one
      if (topLevelItemCount() == 0) {
            emit selectedTrackChanged(nullptr);
            return;
      }

      blockSignals(true);

      if (topLevel == MIXERTREE_INVALID_INDEX || topLevel < 0 || topLevel >=  topLevelItemCount()) {
            // nothing saved or the saved row is out of range - go to the top of the tree
            setCurrentItem(this->itemAt(0,0));
            }
      else {

            QTreeWidgetItem* itemOrItsParent = topLevelItem(topLevel);

            if (!itemOrItsParent) {
                  // the saved row is out of range - go to the top of the tree
                  setCurrentItem(this->itemAt(0,0));
;                 }
            else {
                  if (savedSelectionChildIndex == MIXERTREE_INVALID_INDEX
                      || savedSelectionChildIndex >= itemOrItsParent->childCount()) {
                        // the child row is off, fall back to parent
                        setCurrentItem(itemOrItsParent);
                        }
                  else {
                        // the saved index is valid, so use it
                        setCurrentItem(itemOrItsParent->child(savedSelectionChildIndex));
                        }
                  }
            }
      MixerTrackItem* newMixerTrackItem = static_cast<MixerTreeWidgetItem*>(currentItem())->mixerTrackItem();

      blockSignals(false);
      emit selectedTrackChanged(newMixerTrackItem);
      }



void MixerTreeWidget::saveTreeSelection()
      {
      QTreeWidgetItem* item = currentItem();

      if (!item) {
            savedSelectionTopLevelIndex = MIXERTREE_INVALID_INDEX;
            return;
            }

      savedSelectionTopLevelIndex = indexOfTopLevelItem(item);
      if (savedSelectionTopLevelIndex != MIXERTREE_INVALID_INDEX) {
            // current selection is a top level item
            savedSelectionChildIndex = MIXERTREE_INVALID_INDEX;
            return;
            }

      QTreeWidgetItem* parentOfCurrentItem = item->parent();

      savedSelectionTopLevelIndex = indexOfTopLevelItem(parentOfCurrentItem);
      savedSelectionChildIndex = parentOfCurrentItem->indexOfChild(item);
      }


} // namespace Ms
