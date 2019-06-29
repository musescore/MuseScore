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
//#include "parteditbase.h"

#include "libmscore/excerpt.h"
#include "libmscore/score.h"
#include "libmscore/part.h"
#include "mixer.h"
#include "seq.h"
#include "libmscore/undo.h"
#include "synthcontrol.h"
#include "synthesizer/msynthesizer.h"
#include "preferences.h"
#include <QtGlobal>
#include <qmessagebox.h>
#include <accessibletoolbutton.h>
#include "mixerdetails.h"
#include "mixertrack.h"
#include "mixertrackchannel.h"
#include "mixertrackpart.h"
#include "mixertrackitem.h"
#include "awl/fastlog.h"


namespace Ms {

#define _setValue(__x, __y) \
      __x->blockSignals(true); \
      __x->setValue(__y); \
      __x->blockSignals(false);

#define _setChecked(__x, __y) \
      __x->blockSignals(true); \
      __x->setChecked(__y); \
      __x->blockSignals(false);


double volumeToUserRange(char v) { return v * 100.0 / 128.0; }
double panToUserRange(char v) { return (v / 128.0) * 360.0; }
double chorusToUserRange(char v) { return v * 100.0 / 128.0; }
double reverbToUserRange(char v) { return v * 100.0 / 128.0; }

const float minDecibels = -3;

//0 to 100
char userRangeToVolume(double v) { return (char)qBound(0, (int)(v / 100.0 * 128.0), 127); }
//-180 to 180
char userRangeToPan(double v) { return (char)qBound(0, (int)((v / 360.0) * 128.0), 127); }
//0 to 100
char userRangeToChorus(double v) { return (char)qBound(0, (int)(v / 100.0 * 128.0), 127); }
//0 to 100
char userRangeToReverb(double v) { return (char)qBound(0, (int)(v / 100.0 * 128.0), 127); }

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
      contextMenu = new MixerContextMenu(this);

      qDebug()<<"option showingDetails() = "<<options->showingDetails();

      showDetails(options->showingDetails());

      keyboardFilter = new MixerKeyboardControlFilter(this);
      this->installEventFilter(keyboardFilter);
      mixerTreeWidget->installEventFilter(keyboardFilter);

      savedSelectionTopLevelIndex = -1;   // no saved selection (bit of a magic number :( )
      showDetailsButton->setChecked(options->showingDetails());

      enablePlay = new EnablePlayForWidget(this);
      retranslate(true);
      setupSlotsAndSignals();
      updateTracks();
      updateUiOptions();
      }


void Mixer::setupSlotsAndSignals()
      {
      connect(mixerTreeWidget,SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),SLOT(currentMixerTreeItemChanged()));
      connect(synti,SIGNAL(gainChanged(float)),SLOT(synthGainChanged(float)));
      connect(showDetailsButton,SIGNAL(clicked()),SLOT(showDetailsClicked()));

      connect(mixerTreeWidget->header(), SIGNAL(geometriesChanged()), this, SLOT(adjustHeaderWidths()));
      }

void Mixer::setupAdditionalUi()
      {
      //setup the master channel widget (volume control and Play and Loop button)

      masterChannelWidget = new MixerMasterChannel();
      masterVolumeTreeWidget->clear();
      QTreeWidgetItem* masterVolumeItem = new QTreeWidgetItem(masterVolumeTreeWidget);
      masterVolumeItem->setText(0, "Master");
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
      mixerTreeWidget->header()->setSectionResizeMode(0, QHeaderView::Fixed);
      mixerTreeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);

      mixerTreeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
      adjustHeaderWidths();
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
      setWindowTitle(tr("Mixer"));  //TODO: translate instrument / channel names
      //      if (!firstTime) {
      //            for (int i = 0; i < trackAreaLayout->count(); i++) {
      //                  PartEdit* p = getPartAtIndex(i);
      //                  if (p) p->retranslateUi(p);
      //                  }
      //            }
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
      qDebug()<<"Mixer::setPlayBackScore";
      if (_score != score) {
            _score = score;
            //mixerDetails->setTrack(0);
            }
      updateTracks();
      }



//MARK:- user actions (including context menu)
void Mixer::showDetailsBelow()
      {
      qDebug()<<"showDetailsBelow toggle -  menu item triggered.";
      options->setDetailsOnTheSide(contextMenu->detailToSide->isChecked());
      updateUiOptions();
      }

void Mixer::showMidiOptions()
      {
      qDebug()<<"Show/Hide Midi toggle - menu action triggered.";
      options->setMidiOptions(contextMenu->showMidiOptions->isChecked());
      updateUiOptions();
      }

void Mixer::showTrackColors()
      {
      options->setTrackColors(contextMenu->showTrackColors->isChecked());
      updateUiOptions();
      }

void Mixer::showMasterVolume()
      {
      options->setShowMasterVolume(contextMenu->showMasterVolume->isChecked());
      updateUiOptions();
      }

void Mixer::updateVolumeMode()
      {
      if (contextMenu->overallVolumeOverrideMode->isChecked())
            options->setMode(MixerVolumeMode::Override);

      if (contextMenu->overallVolumeRatioMode->isChecked())
            options->setMode(MixerVolumeMode::Ratio);

      if (contextMenu->overallVolumeFirstMode->isChecked())
            options->setMode(MixerVolumeMode::PrimaryInstrument);
      }

void Mixer::updateUiOptions()
      {
      mixerDetails->setVisible(options->showingDetails());

      QIcon showDetailsIcon;
      bool showMasterVol = options->showMasterVolume();

      if (options->showDetailsOnTheSide()) {
            // show TO THE SIDE case
            showDetailsIcon.addFile(QString::fromUtf8(":/data/icons/go-next.svg"), QSize(), QIcon::Normal, QIcon::Off);

            // addWidget(row, column, rowSpan, columnSpan, [Qt::Alignment])
            gridLayout->addWidget(mixerTreeWidget, 0, 0, 1, 2);
            if (showMasterVol) {
                  gridLayout->addWidget(masterVolumeTreeWidget, 1, 0, 1, 2);
                  masterVolumeTreeWidget->setVisible(true);
                  }
            else {
                  masterVolumeTreeWidget->setVisible(false);
                  }
            gridLayout->addWidget(partOnlyCheckBox, showMasterVol ? 2 : 1, 0, 1, 1);
            gridLayout->addWidget(showDetailsButton, showMasterVol ? 2 : 1, 1, 1, 1);
            gridLayout->addWidget(mixerDetails, 0, 3, showMasterVol ? 3 : 2, 1, Qt::AlignTop);
            }
      else {
            // show BELOW case
            showDetailsIcon.addFile(QString::fromUtf8(":/data/icons/arrow_down.svg"), QSize(), QIcon::Normal, QIcon::Off);
            gridLayout->addWidget(mixerTreeWidget, 0, 0, 1, 2);
            if (showMasterVol) {
                  gridLayout->addWidget(masterVolumeTreeWidget, 1, 0, 1, 2);
                  masterVolumeTreeWidget->setVisible(true);
                  }
            else {
                  masterVolumeTreeWidget->setVisible(false);
                  }
            gridLayout->addWidget(partOnlyCheckBox, showMasterVol ? 2 : 1, 0, 1, 1);
            gridLayout->addWidget(showDetailsButton, showMasterVol ? 2 : 1, 1, 1, 1);
            gridLayout->addWidget(mixerDetails, showMasterVol ? 3 : 2, 0, 1 , 2, Qt::AlignTop);
            }

      showDetailsButton->setIcon(showDetailsIcon);


      // track colors and what is shown in the details list
      mixerDetails->updateUiOptions();

      // track colors in the main mixer
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

      // layout of master volume (is affected by presence or absence or track color
      masterChannelWidget->updateUiControls();

      }

void Mixer::contextMenuEvent(QContextMenuEvent *event)
      {
      contextMenu->contextMenuEvent(event);
      }

      
void Mixer::showDetailsClicked()
      {
      bool visible = !mixerDetails->isVisible();
      options->setShowingDetails(visible);
      showDetails(visible);
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


//MARK:- listen to changes from elsewhere
//---------------------------------------------------------
//   synthGainChanged
//---------------------------------------------------------

void Mixer::synthGainChanged(float)
      {
      float decibels = qBound(minDecibels, log10f(synti->gain()), 0.0f);
      //qDebug()<<"Mixer::synthGainChanged(). Some maths required for sensible slider operation (synti->gain = "<<synti->gain()<<" decibels = "<<decibels<<" (?)";
      //TODO: maths required for sensible slider operation
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

void Mixer::midiPrefsChanged(bool)
      {
      qDebug()<<"Mixer::midiPrefsChanged";
      updateTracks();
      }


//MARK:- event handling

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


//---------------------------------------------------------
//   eventFilter
//---------------------------------------------------------

bool Mixer::eventFilter(QObject* object, QEvent* event)
      {
      if (enablePlay->eventFilter(object, event))
            return true;

      if (object == mixerDetails->panSlider) {
            if (event->type() == QEvent::MouseButtonDblClick) {
                  QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
                  qDebug() << "Ate Double click on pan slider" << keyEvent->key();
                  mixerDetails->resetPanToCentre();
                  return true;
                  }
            }

      return QWidget::eventFilter(object, event);
      }


bool MixerKeyboardControlFilter::eventFilter(QObject *obj, QEvent *event)
      {
      
      MixerTrackItem* selectedMixerTrackItem = mixer->mixerDetails->getSelectedMixerTrackItem();
      
      if (event->type() != QEvent::KeyPress) {
            return QObject::eventFilter(obj, event);
            }
      
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
      
      if (keyEvent->key() == Qt::Key_Period && keyEvent->modifiers() == Qt::NoModifier) {
            qDebug()<<"Volume up keyboard command";
            if (selectedMixerTrackItem && int(selectedMixerTrackItem->getVolume()) < 128) {
                  selectedMixerTrackItem->setVolume(selectedMixerTrackItem->getVolume() + 1);
                  }
            return true;
            }
      
      if (keyEvent->key() == Qt::Key_Comma && keyEvent->modifiers() == Qt::NoModifier) {
            qDebug()<<"Volume down keyboard command";
            if (selectedMixerTrackItem && int(selectedMixerTrackItem->getVolume()) >0) {
                  selectedMixerTrackItem->setVolume(selectedMixerTrackItem->getVolume() - 1);
                  }
            return true;
            }
      
      if (keyEvent->key() == Qt::Key_Less && keyEvent->modifiers() == Qt::ShiftModifier) {
            qDebug()<<"Pan left keyboard command";
            if (selectedMixerTrackItem && int(selectedMixerTrackItem->getPan()) < 128) {
                  selectedMixerTrackItem->setPan(selectedMixerTrackItem->getPan() + 1);
                  }
            return true;
            }
      
      if (keyEvent->key() == Qt::Key_Greater && keyEvent->modifiers() == Qt::ShiftModifier) {
            qDebug()<<"Pan right keyboard command";
            if (selectedMixerTrackItem && int(selectedMixerTrackItem->getPan()) >0) {
                  selectedMixerTrackItem->setPan(selectedMixerTrackItem->getPan() - 1);
                  }
            return true;
            }
      
      if (keyEvent->key() == Qt::Key_M && keyEvent->modifiers() == Qt::NoModifier) {
            qDebug()<<"Mute (M) keyboard command";
            if (selectedMixerTrackItem) {
                  selectedMixerTrackItem->setMute(!selectedMixerTrackItem->getMute());
                  }
            return true;
            }
      
      if (keyEvent->key() == Qt::Key_S && keyEvent->modifiers() == Qt::NoModifier) {
            qDebug()<<"Solo (S) keyboard command";
            if (selectedMixerTrackItem) {
                  selectedMixerTrackItem->setSolo(!selectedMixerTrackItem->getSolo());
                  }
            return true;
            }
      
      return QObject::eventFilter(obj, event);
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

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void Mixer::changeEvent(QEvent *event)
      {
      QWidget::changeEvent(event);
      if (event->type() == QEvent::LanguageChange)
            retranslate();
      }

//---------------------------------------------------------
//   partEdit
//---------------------------------------------------------
//
//PartEdit* Mixer::getPartAtIndex(int)
//      {
//      return 0;
//      }



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
            // When it's created the item will also reate any children and setup their widgets
            MixerTreeWidgetItem* item = new MixerTreeWidgetItem(part, _score, mixerTreeWidget);
            mixerTreeWidget->addTopLevelItem(item);
            mixerTreeWidget->setItemWidget(item, 1, new MixerTrackChannel(item));
            }

      // TODO: need re-implement remembering expansion state and fix unwanted display bug

      if (savedSelectionTopLevelIndex == -1 && mixerTreeWidget->topLevelItemCount() > 0) {
            mixerTreeWidget->setCurrentItem(mixerTreeWidget->itemAt(0,0));
            currentMixerTreeItemChanged();
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
      mixerDetails->updateDetails(item->mixerTrackItem);
      }

//MARK:- support classes

MixerKeyboardControlFilter::MixerKeyboardControlFilter(Mixer* mixer) : mixer(mixer)
      {
      }


MixerOptions::MixerOptions()
      {
      readSettings();
      }

void MixerOptions::readSettings() {

      QSettings settings("MuseScore","Mixer");
      settings.beginGroup("MixerOptions");
      _showTrackColors = settings.value("showTrackColors", true).toBool();
      _detailsOnTheSide = settings.value("detailsOnTheSide", false).toBool();
      _showMidiOptions = settings.value("showMidiOptions", true).toBool();
      _showingDetails = settings.value("showingDetails", true).toBool();
      _showMasterVolume = settings.value("showMasterVolume", true).toBool();
      int rawMode = settings.value("sliderMode", 1).toInt();
      settings.endGroup();

      // hack because C++ documentation is horrible and I want it done
      // fix up later
      MixerVolumeMode convertEnum;

      switch (rawMode) {
            case 1:
                  convertEnum = MixerVolumeMode::Override;
                  break;
            case 2:
                  convertEnum = MixerVolumeMode::Ratio;
                  break;
            case 3:
                  convertEnum = MixerVolumeMode::PrimaryInstrument;
                  break;
            default:
                  convertEnum = MixerVolumeMode::Override;
            }

      _mode = convertEnum;

      }

void MixerOptions::writeSettings() {
      QSettings settings("MuseScore","Mixer");
      settings.beginGroup("MixerOptions");
      settings.setValue("showTrackColors", _showTrackColors);
      settings.setValue("detailsOnTheSide", _detailsOnTheSide);
      settings.setValue("showMidiOptions", _showMidiOptions);
      settings.setValue("showingDetails", _showingDetails);
      settings.setValue("showMasterVolume", _showMasterVolume);

      qDebug()<<"write settings _showingDetails = "<<_showingDetails;

      // hack because C++ documentation is horrible and I want it done
      // fix up later
      int convertEnum = 0;
      switch (_mode) {
            case MixerVolumeMode::Override:
                  convertEnum = 1;
                  break;
            case MixerVolumeMode::Ratio:
                  convertEnum = 2;
                  break;
            case MixerVolumeMode::PrimaryInstrument:
                  convertEnum = 3;
                  break;
            }

      settings.setValue("sliderMode", convertEnum);
      settings.endGroup();
      }

MixerContextMenu::MixerContextMenu(Mixer* mixer) : mixer(mixer)
      {
      MixerOptions* options = Mixer::getOptions();

      detailToSide = new QAction(tr("Show Details to the Side"));
      detailToSide->setCheckable(true);
      detailToSide->setChecked(options->showDetailsOnTheSide());
      showMidiOptions = new QAction(tr("Show Midi Options"));
      showMidiOptions->setCheckable(true);
      showMidiOptions->setChecked(options->showMidiOptions());
      panSliderInMixer = new QAction(tr("Show Pan Slider in Mixer (not impl yet)"));

      overallVolumeOverrideMode = new QAction(tr("Overall volume: override"));
      overallVolumeRatioMode = new QAction(tr("Overall volume: relative"));
      overallVolumeFirstMode = new QAction(tr("Overall volume: primary only"));

      overallVolumeOverrideMode->setCheckable(true);
      overallVolumeRatioMode->setCheckable(true);
      overallVolumeFirstMode->setCheckable(true);

      overallVolumeOverrideMode->setChecked(options->mode() == MixerVolumeMode::Override);
      overallVolumeRatioMode->setChecked(options->mode() == MixerVolumeMode::Ratio);
      overallVolumeFirstMode->setChecked(options->mode() == MixerVolumeMode::PrimaryInstrument);

      modeGroup = new QActionGroup(mixer);

      modeGroup->addAction(overallVolumeOverrideMode);
      modeGroup->addAction(overallVolumeRatioMode);
      modeGroup->addAction(overallVolumeFirstMode);

      showTrackColors = new QAction(tr("Show Track Colors"));
      showTrackColors->setCheckable(true);
      showMasterVolume = new QAction(tr("Show Master Volume"));
      showMasterVolume->setCheckable(true);
      showMasterVolume->setChecked(options->showMasterVolume());

      detailToSide->setStatusTip(tr("Detailed options shown below the mixer"));
      connect(detailToSide, SIGNAL(changed()), mixer, SLOT(showDetailsBelow()));
      connect(showTrackColors, SIGNAL(changed()), mixer, SLOT(showTrackColors()));
      connect(showMidiOptions, SIGNAL(changed()), mixer, SLOT(showMidiOptions()));
      connect(showMasterVolume, SIGNAL(changed()), mixer, SLOT(showMasterVolume()));

      connect(overallVolumeOverrideMode, SIGNAL(changed()), mixer, SLOT(updateVolumeMode()));
      connect(overallVolumeRatioMode, SIGNAL(changed()), mixer, SLOT(updateVolumeMode()));
      connect(overallVolumeFirstMode, SIGNAL(changed()), mixer, SLOT(updateVolumeMode()));
      }

void MixerContextMenu::contextMenuEvent(QContextMenuEvent *event)
      {
      QMenu menu(mixer);
      menu.addSection(tr("Customize"));
      menu.addAction(detailToSide);
      menu.addSeparator();
      menu.addAction(showMidiOptions);
      menu.addAction(showTrackColors);
      menu.addAction(showMasterVolume);
      menu.addSection(tr("Secondary Slider"));
      menu.addAction(panSliderInMixer);
      menu.addSection(tr("Slider Behavior"));
      menu.addAction(overallVolumeFirstMode);
      menu.addAction(overallVolumeOverrideMode);
      menu.addAction(overallVolumeRatioMode);
      menu.exec(event->globalPos());
      }

}
