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

#include "musescore.h"

#include "libmscore/excerpt.h"
#include "libmscore/score.h"
#include "libmscore/part.h"
#include "mixer.h"
#include "seq.h"
#include "libmscore/undo.h"
#include "synthcontrol.h"
#include "synthesizer/msynthesizer.h"
#include "preferences.h"

#include <accessibletoolbutton.h>

#include "mixerdetails.h"
#include "mixertrackchannel.h"
#include "mixermasterchannel.h"
#include "mixertrackitem.h"
#include "mixertreewidgetitem.h"
#include "mixeroptions.h"
#include "mixeroptionsbutton.h"

namespace Ms {

#define _setValue(__x, __y) \
      __x->blockSignals(true); \
      __x->setValue(__y); \
      __x->blockSignals(false);

#define _setChecked(__x, __y) \
      __x->blockSignals(true); \
      __x->setChecked(__y); \
      __x->blockSignals(false);


// initialise the static
MixerOptions* Mixer::options = new MixerOptions(); // will read from settings

//---------------------------------------------------------
//   Mixer
//---------------------------------------------------------
//MARK:- create and setup
Mixer::Mixer(QWidget* parent)
      : QDockWidget("Mixer", parent)
      {

      setupUi(this);

      setWindowFlags(Qt::Tool);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      setupAdditionalUi();

      gridLayout = new QGridLayout(dockWidgetContents);
      mixerDetails = new MixerDetails(this);

      showDetails(options->showingDetails());

      keyboardFilter = new MixerKeyboardControlFilter(this);
      this->installEventFilter(keyboardFilter);
      mixerTreeWidget->installEventFilter(keyboardFilter);

      savedSelectionTopLevelIndex = -1;   // no saved selection (bit of a magic number :( )

      enablePlay = new EnablePlayForWidget(this);
      setupSlotsAndSignals();
      updateTracks();
      updateUiOptions();
      retranslate(true);

      shiftKeyMonitorTimer = new QTimer(this);
      connect(shiftKeyMonitorTimer, SIGNAL(timeout()), this, SLOT(shiftKeyMonitor()));
      shiftKeyMonitorTimer->start(100);
      }




void Mixer::setupSlotsAndSignals()
      {
      connect(mixerTreeWidget,SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),SLOT(currentMixerTreeItemChanged()));
      connect(synti,SIGNAL(gainChanged(float)),SLOT(synthGainChanged(float)));

      connect(mixerTreeWidget->header(), SIGNAL(geometriesChanged()), this, SLOT(adjustHeaderWidths()));
      connect(mixerTreeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(itemCollapsedOrExpanded(QTreeWidgetItem*)));
      connect(mixerTreeWidget, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(itemCollapsedOrExpanded(QTreeWidgetItem*)));
      connect(mixerTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(itemChanged(QTreeWidgetItem*, int)));

      }


void Mixer::setupAdditionalUi()
      {
      //setup the master channel widget (volume control and Play and Loop button)

      masterChannelWidget = new MixerMasterChannel();
      masterVolumeTreeWidget->clear();
      QTreeWidgetItem* masterVolumeItem = new QTreeWidgetItem(masterVolumeTreeWidget);
      masterVolumeItem->setText(0, tr("Master"));
      masterVolumeTreeWidget->addTopLevelItem(masterVolumeItem);
      masterVolumeTreeWidget->setItemWidget(masterVolumeItem, 1, masterChannelWidget);

      masterVolumeTreeWidget->setColumnCount(2);
      masterVolumeTreeWidget->header()->setSectionResizeMode(0, QHeaderView::Fixed);
      masterVolumeTreeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);
      masterVolumeTreeWidget->setSelectionMode(QAbstractItemView::NoSelection);

      // configure the main mixer tree
      mixerTreeWidget->setAlternatingRowColors(true);
      mixerTreeWidget->setColumnCount(2);
      mixerTreeWidget->setHeaderLabels({tr("Instrument"), tr("Volume")});
      // make all bar column 0, non-editable
      mixerTreeWidget->setItemDelegateForColumn(1, new NonEditableItemDelegate (mixerTreeWidget));

      mixerTreeWidget->header()->setSectionResizeMode(0, QHeaderView::Fixed);
      mixerTreeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);

      mixerTreeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
      adjustHeaderWidths();

      showDetailsButton->setTarget(this);
      }


void Mixer::adjustHeaderWidths()
      {
      int width = mixerTreeWidget->width();

      int firstColumnMaximumWidth = 300;
      double ratio = 0.4; // instrument names take up 2/5 and controls 3/5
      int margin = 2;   // factor to avoid triggering horizontal scrolling

      int column0 = int(double(width) * ratio);
      int column1 = int(double(width) * (1-ratio) - margin);

      if (column0 > firstColumnMaximumWidth) {
            column0 = firstColumnMaximumWidth;
            column1 = (width - firstColumnMaximumWidth) - margin;
            }

      mixerTreeWidget->header()->resizeSection(0, column0);
      mixerTreeWidget->header()->resizeSection(1, column1);
      masterVolumeTreeWidget->header()->resizeSection(0, column0);
      masterVolumeTreeWidget->header()->resizeSection(1, column1);
      }

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void Mixer::retranslate(bool firstTime)
      {
      setWindowTitle(tr("Mixer"));
      if (firstTime)
            return;

      retranslateUi(this);
      mixerDetails->retranslateUi(mixerDetails);
      updateMixerTreeHeaders();
      //TODO: retranslate instrument names (but do they have translations)
      }


//MARK:- main interface

void MuseScore::showMixer(bool visible)
      {
      QAction* toggleMixerAction = getAction("toggle-mixer");
      if (mixer == 0) {
            mixer = new Mixer(this);
            mscore->stackUnder(mixer);
            if (synthControl)
                  connect(synthControl, SIGNAL(soundFontChanged()), mixer, SLOT(updateTrack()));
            connect(synti, SIGNAL(soundFontChanged()), mixer, SLOT(updateTracks()));
            connect(mixer, SIGNAL(closed(bool)), toggleMixerAction, SLOT(setChecked(bool)));
            mixer->setFloating(false);
            addDockWidget(Qt::RightDockWidgetArea, mixer);
            }
      reDisplayDockWidget(mixer, visible);
      toggleMixerAction->setChecked(visible);
      mixer->setScore(cs);
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Mixer::setScore(Score* score)
      {
      // No equality check, this function seems to need to cause
      // mixer update every time it gets called.
      _activeScore = score;
      setPlaybackScore(_activeScore ? _activeScore->masterScore()->playbackScore() : nullptr);

      partOnlyCheckBox->setChecked(mscore->playPartOnly());
      partOnlyCheckBox->setEnabled(_activeScore && !_activeScore->isMaster());
      }

//---------------------------------------------------------
//   setPlaybackScore
//---------------------------------------------------------

void Mixer::setPlaybackScore(Score* score)
      {
      if (_score != score) {
            _score = score;
            //mixerDetails->setTrack(0);
            }
      updateTracks();
      }


void Mixer::updateTreeSlidersAppearance()
      {
      // secondary slider style and track colors in the main mixer
      for (int topLevelIndex = 0; topLevelIndex < mixerTreeWidget->topLevelItemCount(); topLevelIndex++) {
            QTreeWidgetItem* topLevelItem = mixerTreeWidget->topLevelItem(topLevelIndex);
            MixerTrackChannel* itemWidget = static_cast<MixerTrackChannel*>(mixerTreeWidget->itemWidget(topLevelItem, 1));
            itemWidget->updateUiControls();

            for (int childIndex = 0; childIndex < topLevelItem->childCount(); childIndex++) {
                  QTreeWidgetItem* childItem = topLevelItem->child(childIndex);
                  MixerTrackChannel* itemWidget = static_cast<MixerTrackChannel*>(mixerTreeWidget->itemWidget(childItem, 1));
                  itemWidget->updateUiControls();
            }
      }
      }



void Mixer::showDetails(bool visible)
      {
      QSize currentTreeWidgetSize = mixerTreeWidget->size();
      QSize minTreeWidgetSize = mixerTreeWidget->minimumSize();   // respect settings from QT Creator / QT Designer
      QSize maxTreeWidgetSize = mixerTreeWidget->maximumSize();   // respect settings from QT Creator / QT Designer

      if (!isFloating() && visible) {
            // Special case - make the mixerTreeView as narrow as possible before showing the
            // detailsView. Without this step, mixerTreeView will be as fully wide as the dock
            // and when the detailsView is added it will get even wider. (And if the user toggles QT
            // will keep making the dock / mainWindow wider and wider, which is highly undesirable.)
            mixerTreeWidget->setMaximumSize(minTreeWidgetSize);
            mixerDetails->setVisible(visible);
            dockWidgetContents->adjustSize();
            mixerTreeWidget->setMaximumSize(maxTreeWidgetSize);
            return;
            }

      // Pin the size of the mixerView when either showing or hiding the details view.
      // This ensures that the mixer window (when undocked) will shrink or grow as
      // appropriate.
      mixerTreeWidget->setMinimumSize(currentTreeWidgetSize);
      mixerTreeWidget->setMaximumSize(currentTreeWidgetSize);
      mixerDetails->setVisible(visible);
      mixerTreeWidget->adjustSize();
      dockWidgetContents->adjustSize();
      this->adjustSize(); // All three adjustSize() calls (appear) to be required
      mixerTreeWidget->setMinimumSize(minTreeWidgetSize);
      mixerTreeWidget->setMaximumSize(maxTreeWidgetSize);
      }


void Mixer::enterSecondarySliderMode(bool enter)
      {
      options->setSecondaryModeOn(enter);
      updateMixerTreeHeaders();
      updateTreeSlidersAppearance();
      }


//---------------------------------------------------------
//   on_partOnlyCheckBox_toggled
//---------------------------------------------------------

void Mixer::on_partOnlyCheckBox_toggled(bool checked)
      {

      if (!_activeScore || !_activeScore->excerpt())
            return;

      mscore->setPlayPartOnly(checked);
      setPlaybackScore(_activeScore->masterScore()->playbackScore());

      // Prevent muted channels from sounding
      for (const MidiMapping& mm : _activeScore->masterScore()->midiMapping()) {
            const Channel* ch = mm.articulation();
            if (ch && (ch->mute() || ch->soloMute()))
                  seq->stopNotes(ch->channel());
            }
      }

//MARK:- update ui

void Mixer::updateUiOptions()
{
      // track colors and what is shown in the details list
      updateTreeSlidersAppearance();
      mixerDetails->updateUiOptions();

      // layout of master volume (is affected by presence or absence or track color
      masterChannelWidget->updateUiControls();

      showDetails(options->showingDetails());

      bool showMasterVol = options->showMasterVolume();

      if (options->showDetailsOnTheSide()) {
            // show TO THE SIDE case

            // addWidget(row, column, rowSpan, columnSpan, [Qt::Alignment])
            gridLayout->addWidget(partOnlyCheckBox, 0, 1, 1, 1, Qt::AlignRight);
            gridLayout->addWidget(showDetailsButton, 0, 0, 1, 1);
            gridLayout->addWidget(mixerTreeWidget, 1, 0, 1, 2);
            if (showMasterVol) {
                  gridLayout->addWidget(masterVolumeTreeWidget, 2, 0, 1, 2);
                  masterVolumeTreeWidget->setVisible(true);
            }
            else {
                  masterVolumeTreeWidget->setVisible(false);
            }

            gridLayout->addWidget(mixerDetails, 0, 2, showMasterVol ? 3 : 2, 1, Qt::AlignTop);
      }
      else {
            // show BELOW case

            // addWidget(row, column, rowSpan, columnSpan, [Qt::Alignment])
            gridLayout->addWidget(partOnlyCheckBox, 0, 1, 1, 1, Qt::AlignRight);
            gridLayout->addWidget(showDetailsButton, 0, 0, 1, 1);
            gridLayout->addWidget(mixerTreeWidget, 1, 0, 1, 2);
            if (showMasterVol) {
                  gridLayout->addWidget(masterVolumeTreeWidget, 2, 0, 1, 2);
                  masterVolumeTreeWidget->setVisible(true);
            }
            else {
                  masterVolumeTreeWidget->setVisible(false);
            }

            gridLayout->addWidget(mixerDetails, showMasterVol ? 3 : 2, 0, 1 , 2, Qt::AlignTop);

            gridLayout->setRowStretch(1,10);
      }

      // cover case where the LOCK has changed (but there's no change in SHIFT key)
      enterSecondarySliderMode(options->secondaryModeOn());
}


void Mixer::updateMixerTreeHeaders() {

      if (!options->secondaryModeOn()) {
            mixerTreeWidget->setHeaderLabels({tr("Instrument"), tr("Volume")});
            return;
      }

      QString secondary;

      switch (options->secondarySlider()) {
            case MixerOptions::MixerSecondarySlider::Pan:
                  secondary = tr("Pan");
                  break;
            case MixerOptions::MixerSecondarySlider::Reverb:
                  secondary = tr("Reverb");
                  break;
            case MixerOptions::MixerSecondarySlider::Chorus:
                  secondary = tr("Chorus");
      }

      mixerTreeWidget->setHeaderLabels({tr("Instrument"), secondary});
}



//MARK:- listen to changes from elsewhere
//---------------------------------------------------------
//   synthGainChanged
//---------------------------------------------------------

void Mixer::synthGainChanged(float)
      {
      masterChannelWidget->volumeChanged(synti->gain());
      }

//---------------------------------------------------------
//   masterVolumeChanged
//---------------------------------------------------------

void Mixer::masterVolumeChanged(double decibels)
      {
      float gain = qBound(0.0f, powf(10, (float)decibels), 1.0f);
      synti->setGain(gain);
      }


//---------------------------------------------------------
//   midiPrefsChanged
//---------------------------------------------------------

// sent from Preferences dialogue - not clear it did anything in
// previous version of mixer. With this design, might be better
// to remove the preference as it's handled within the mixer. Or,
// perhaps to honour it but also disable the show/hide on the
// dropdown menu - that seems a bit daft though.
//
void Mixer::midiPrefsChanged(bool)
      {
      updateTracks();
      }


//MARK:- window events

void Mixer::closeEvent(QCloseEvent* ev)
      {
      emit closed(false);
      QWidget::closeEvent(ev);
      }

//---------------------------------------------------------
//   showEvent
//---------------------------------------------------------

void Mixer::showEvent(QShowEvent* e)
      {
      enablePlay->showEvent(e);
      QWidget::showEvent(e);
      activateWindow();
      setFocus();
      getAction("toggle-mixer")->setChecked(true);
      }


//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void Mixer::hideEvent(QHideEvent* e)
      {
      QWidget::hideEvent(e);
      getAction("toggle-mixer")->setChecked(false);
      }

//MARK:- keyboard events

//---------------------------------------------------------
//   eventFilter
//---------------------------------------------------------

bool Mixer::eventFilter(QObject* object, QEvent* event)
      {
      if (enablePlay->eventFilter(object, event))
            return true;

      return QWidget::eventFilter(object, event);
      }


bool MixerKeyboardControlFilter::eventFilter(QObject *obj, QEvent *event)
      {

      if (event->type() != QEvent::KeyPress) {
            return QObject::eventFilter(obj, event);
      }

      MixerTrackItem* selectedMixerTrackItem = mixer->mixerDetails->getSelectedMixerTrackItem();

      if (!selectedMixerTrackItem)
            return QObject::eventFilter(obj, event);
      
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

      bool modified = keyEvent->modifiers() == Qt::ShiftModifier;

      bool secondaryLock = Mixer::getOptions()->secondaryModeLock();
      modified = secondaryLock ? !modified : modified;

      Qt::Key primaryDown = !secondaryLock ? Qt::Key_Comma : Qt::Key_Less;
      Qt::Key primaryUp = !secondaryLock ? Qt::Key_Period : Qt::Key_Greater;
      Qt::Key secondaryDown = !secondaryLock ? Qt::Key_Less : Qt::Key_Comma;
      Qt::Key secondaryUp = !secondaryLock ? Qt::Key_Greater : Qt::Key_Period;


      if (keyEvent->key() == primaryDown && !modified) {
            mixer->nudgeMainSlider(Mixer::NudgeDirection::Down);
            return true;
      }
            if (keyEvent->key() == primaryUp && !modified) {
            mixer->nudgeMainSlider(Mixer::NudgeDirection::Up);
            return true;
            }

      
      if (keyEvent->key() == secondaryDown && modified) {
            mixer->nudgeSecondarySlider(Mixer::NudgeDirection::Down);
            return true;
            }
      
      if (keyEvent->key() == secondaryUp && modified) {
            mixer->nudgeSecondarySlider(Mixer::NudgeDirection::Up);
            return true;
            }
      
      if (keyEvent->key() == Qt::Key_M && keyEvent->modifiers() == Qt::NoModifier) {
            if (selectedMixerTrackItem) {
                  selectedMixerTrackItem->setMute(!selectedMixerTrackItem->getMute());
                  }
            return true;
            }
      
      if (keyEvent->key() == Qt::Key_S && keyEvent->modifiers() == Qt::NoModifier) {
            if (selectedMixerTrackItem) {
                  selectedMixerTrackItem->setSolo(!selectedMixerTrackItem->getSolo());
                  }
            return true;
            }
      
      return QObject::eventFilter(obj, event);
      }

int Mixer::nudge(int currentValue, NudgeDirection direction, int lowerLimit, int upperLimit) {

            int proposedValue = currentValue + (direction == NudgeDirection::Up ? 1 : -1);

            if (currentValue > upperLimit)
                  proposedValue = upperLimit;

            if (currentValue < lowerLimit)
                  proposedValue = lowerLimit;

            return proposedValue;
      }

void Mixer::nudgeMainSlider(NudgeDirection direction)
      {
      MixerTrackItem* trackItem = mixerDetails->getSelectedMixerTrackItem();
      int proposedValue = nudge(trackItem->getVolume(), direction, 0, 127);
      int acceptedValue = trackItem->setVolume(proposedValue);

      if (proposedValue != acceptedValue) {
            QApplication::beep();
            }
      }

void Mixer::nudgeSecondarySlider(NudgeDirection direction)
      {
      MixerTrackItem* trackItem = mixerDetails->getSelectedMixerTrackItem();

      int proposedValue;

      switch (options->secondarySlider()) {
            case MixerOptions::MixerSecondarySlider::Pan:
                  proposedValue = nudge(trackItem->getPan(), direction, -63, 63);
                  break;
            case MixerOptions::MixerSecondarySlider::Reverb:
                  proposedValue = nudge(trackItem->getReverb(), direction, 0, 127);
                  break;
            case MixerOptions::MixerSecondarySlider::Chorus:
                  proposedValue = nudge(trackItem->getChorus(), direction, 0, 127);
                  break;
      }

      int acceptedValue;
      switch (options->secondarySlider()) {
            case MixerOptions::MixerSecondarySlider::Pan:
                  acceptedValue = trackItem->setPan(proposedValue);
                  break;
            case MixerOptions::MixerSecondarySlider::Reverb:
                  acceptedValue = trackItem->setReverb(proposedValue);
                  break;
            case MixerOptions::MixerSecondarySlider::Chorus:
                  acceptedValue = trackItem->setChorus(proposedValue);
                  break;
      }

      if (proposedValue != acceptedValue) {
            QApplication::beep();
            }

      }

//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

void Mixer::keyPressEvent(QKeyEvent* ev) {
      if (ev->key() == Qt::Key_Escape && ev->modifiers() == Qt::NoModifier) {
            close();
            return;
            }
      QWidget::keyPressEvent(ev);
      }


void Mixer::shiftKeyMonitor() {

      // check if we or any children have the focus
      bool focus = hasFocus();
      if (!focus) {
            QWidget* focusWidget = QApplication::focusWidget();
            focus = focusWidget ? this->isAncestorOf(focusWidget): false;

            // but now check if what's got the focus is some kind of text
            // box, i.e. a spinbox or a lineedit box

            if (dynamic_cast<QLineEdit*>(focusWidget))
                  focus = false;

            if (dynamic_cast<QSpinBox*>(focusWidget))
                  focus = false;

            if (dynamic_cast<QDoubleSpinBox*>(focusWidget))
                  focus = false;

      }

      // if not focus but secondary mode is on, turn it off
      if (!focus) {
            if (options->secondaryModeOn())
                  enterSecondarySliderMode(false);
            return;
      }

      // if shift key is down enter secondary mode (if not in it already)
      // BUT swap this logic if secondaryModeLock() is true
      bool shiftedModeActive = options->secondaryModeLock() ? !options->secondaryModeOn() : options->secondaryModeOn();

      if (QApplication::queryKeyboardModifiers() & Qt::KeyboardModifier::ShiftModifier) {
            if (!shiftedModeActive)
                  enterSecondarySliderMode(true);
            return;
      }

      if (shiftedModeActive)
            enterSecondarySliderMode(false);
}

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void Mixer::changeEvent(QEvent *event)
      {
      QWidget::changeEvent(event);
      if (event->type() == QEvent::LanguageChange)
            retranslate();
      }


//MARK:- tree changes

void Mixer::itemChanged(QTreeWidgetItem* treeWidgetItem, int column)
{
      MixerTreeWidgetItem* item = static_cast<MixerTreeWidgetItem*>(treeWidgetItem);
      saveTreeSelection();
      item->mixerTrackItem()->setName(item->text(column));
      restoreTreeSelection();
}


//MARK:- manage the mixer tree

//---------------------------------------------------------
//   updateTracks
//---------------------------------------------------------
void Mixer::updateTracks()
      {
      qDebug()<<"Mixer::updateTracks()";
      const QSignalBlocker blockTreeWidgetSignals(mixerTreeWidget);  // block during this method

      mixerTreeWidget->clear();

      if (!_score) {
            disableMixer();
            return;
            }

      for (Part* localPart : _score->parts()) {
            Part* part = localPart->masterPart();
            // When it's created the item will also create any children and setup their widgets
            MixerTreeWidgetItem* item = new MixerTreeWidgetItem(part, _score, mixerTreeWidget);
            mixerTreeWidget->addTopLevelItem(item);
            mixerTreeWidget->setItemWidget(item, 1, item->mixerTrackChannel());
            item->setExpanded(part->isExpanded());
            }

      if (savedSelectionTopLevelIndex == -1 && mixerTreeWidget->topLevelItemCount() > 0) {
            mixerTreeWidget->setCurrentItem(mixerTreeWidget->itemAt(0,0));
            currentMixerTreeItemChanged();
            }

      }

void Mixer::itemCollapsedOrExpanded(QTreeWidgetItem* item) {

      MixerTreeWidgetItem* mixerTreeWidgetItem = static_cast<MixerTreeWidgetItem*>(item);
      MixerTrackItem* mixerTrackItem = mixerTreeWidgetItem->mixerTrackItem();
      if (mixerTrackItem && mixerTrackItem->isPart()) {
            mixerTrackItem->part()->setExpanded(item->isExpanded());
      }
}

void Mixer::disableMixer()
      {
      mixerDetails->setEnabled(false);
      mixerDetails->resetControls();
      }


// Used to save the item currently selected in the tree when performing operations
// such as changing the patch. The way changing patches is implemented is that it
// triggers a new setScore() method on the mixer which, in turn, and of necessity,
// forces the channel strip to built again from scratch. Not clear patch changes
// have to do this, but, currently, they do. This works around that.
      
void Mixer::saveTreeSelection()
      {
      QTreeWidgetItem* item = mixerTreeWidget->currentItem();

      if (!item) {
            savedSelectionTopLevelIndex = -1;
            return;
            }

      savedSelectionTopLevelIndex = mixerTreeWidget->indexOfTopLevelItem(item);
      if (savedSelectionTopLevelIndex != -1) {
            // current selection is a top level item
            savedSelectionChildIndex = -1;
            return;
            }

      QTreeWidgetItem* parentOfCurrentItem = item->parent();

      savedSelectionTopLevelIndex = mixerTreeWidget->indexOfTopLevelItem(parentOfCurrentItem);
      savedSelectionChildIndex = parentOfCurrentItem->indexOfChild(item);
      }


void Mixer::restoreTreeSelection()
      {
      int topLevel = savedSelectionTopLevelIndex;
      savedSelectionTopLevelIndex = -1;   // indicates no selection currently saved

      if (topLevel < 0 || mixerTreeWidget->topLevelItemCount() == 0) {
            qDebug()<<"asked to restore tree selection, but it's not possible (1)";
            return;
            }

      if (topLevel >=  mixerTreeWidget->topLevelItemCount()) {
            qDebug()<<"asked to restore tree selection, but it's not possible (2)";
            return;
            }

      QTreeWidgetItem* itemOrItsParent = mixerTreeWidget->topLevelItem(topLevel);

      if (!itemOrItsParent) {
            qDebug()<<"asked to restore tree selection, but it's not possible (3)";
            return;
            }

      if (savedSelectionChildIndex == -1) {
            mixerTreeWidget->setCurrentItem(itemOrItsParent);
            return;
            }

      if (savedSelectionChildIndex >= itemOrItsParent->childCount()) {
            qDebug()<<"asked to restore tree selection, but it's not possible (4)";
            return;
            }

      mixerTreeWidget->setCurrentItem(itemOrItsParent->child(savedSelectionChildIndex));
      }

// - listen for changes to current item so that the details view can be updated
// also called directly by updateTracks (while signals are disabled)
void Mixer::currentMixerTreeItemChanged()
      {
      if (mixerTreeWidget->topLevelItemCount() == 0 || !mixerTreeWidget->currentItem()) {
            qDebug()<<"Mixer::currentMixerTreeItemChanged called with no items in tree - ignoring)";
            return;
            }

      MixerTreeWidgetItem* item = static_cast<MixerTreeWidgetItem*>(mixerTreeWidget->currentItem());
      mixerDetails->updateDetails(item->mixerTrackItem());
      }

//MARK:- support classes

MixerKeyboardControlFilter::MixerKeyboardControlFilter(Mixer* mixer) : mixer(mixer)
      {
      }



} // namespace Ms
