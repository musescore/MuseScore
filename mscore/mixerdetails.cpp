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

#include "mixerdetails.h"
#include "mixertrackitem.h"
#include "mixeroptions.h"


namespace Ms {

//MARK:- Create and setup
MixerDetails::MixerDetails(Mixer *mixer) :
      QWidget(mixer), mixer(mixer),
      selectedMixerTrackItem(nullptr)
      {
      setupUi(this);
      panSlider->setPanMode(true);              // so fill color begins at centre not at left edge + double click to re-centre

      mutePerVoiceGrid = new QGridLayout();
      mutePerVoiceGrid->setContentsMargins(0, 0, 0, 0);
      mutePerVoiceGrid->setSpacing(7);

      updateUiOptions();                        // show or hide certain controls as per user preferences
      setupSlotsAndSignals();
      updateDetails(selectedMixerTrackItem);    // when called with nullptr will reset and disable all controls
      }


void MixerDetails::setupSlotsAndSignals()
      {
      connect(drumkitCheck,         SIGNAL(toggled(bool)),        SLOT(drumsetCheckboxToggled(bool)));
      connect(patchCombo,           SIGNAL(activated(int)),       SLOT(patchComboEdited(int)));
      connect(volumeSlider,         SIGNAL(valueChanged(int)),    SLOT(volumeSliderMoved(int)));
      connect(volumeSpinBox,        SIGNAL(valueChanged(int)),    SLOT(volumeSpinBoxEdited(int)));
      connect(panSlider,            SIGNAL(valueChanged(int)),    SLOT(panSliderMoved(int)));
      connect(panSpinBox,           SIGNAL(valueChanged(int)),    SLOT(panSpinBoxEdited(int)));
      connect(portSpinBox,          SIGNAL(valueChanged(int)),    SLOT(midiChannelOrPortEdited(int)));
      connect(channelSpinBox,       SIGNAL(valueChanged(int)),    SLOT(midiChannelOrPortEdited(int)));
      connect(chorusSlider,         SIGNAL(valueChanged(int)),    SLOT(chorusSliderMoved(int)));
      connect(chorusSpinBox,        SIGNAL(valueChanged(int)),    SLOT(chorusSpinBoxEdited(int)));
      connect(reverbSlider,         SIGNAL(valueChanged(int)),    SLOT(reverbSliderMoved(int)));
      connect(reverbSpinBox,        SIGNAL(valueChanged(int)),    SLOT(reverbSpinBoxEdited(int)));
      }

void MixerDetails::updateUiOptions()
      {
      MixerOptions* options = Mixer::getOptions();

      bool showMidiOptions = options->showMidiOptions();
      reverbSlider->setVisible(showMidiOptions);
      reverbSpinBox->setVisible(showMidiOptions);
      labelReverb->setVisible(showMidiOptions);
      chorusSlider->setVisible(showMidiOptions);
      chorusSpinBox->setVisible(showMidiOptions);
      labelChorus->setVisible(showMidiOptions);
      labelMidiPort->setVisible(showMidiOptions);
      labelMidiChannel->setVisible(showMidiOptions);
      channelSpinBox->setVisible(showMidiOptions);
      portSpinBox->setVisible(showMidiOptions);

      updateTabOrder();
      adjustSize();
      }


//MARK:- Main interface
void MixerDetails::updateDetails(MixerTrackItem* mixerTrackItem)
      {
      selectedMixerTrackItem = mixerTrackItem;

      if (!selectedMixerTrackItem) {
            resetControls();        // return controls to default / unset state
            setEnabled(false);      // disable controls
            setNotifier(nullptr);   // stop listening to messages from current score/part
            return;
            }

      // setNotifier(channel) zaps previous notifiers and then calls addListener(this).
      // As a listener, this object receives propertyChanged() calls when the channel is
      // changed. This ensures the details view is synced with changes in the tree view.
      setNotifier(selectedMixerTrackItem->channel());

      setEnabled(true);

      blockSignals(true);

      updateName();
      updatePatch();
      updateMutePerVoice();
      updateVolume();
      updatePan();
      updateReverb();
      updateChorus();
      updateMidiChannelAndPort();

      blockSignals(false);
      }



// propertyChanged - we're listening to changes to the channel
// When they occur, this method is called so that we can update
// the UI. Signals sent by the UI control are blocked during the
// update to prevent getting caught in an update loop.
void MixerDetails::propertyChanged(Channel::Prop property)
      {
      if (!selectedMixerTrackItem)
            return;

      blockSignals(true);

      switch (property) {
            case Channel::Prop::VOLUME: {
                  updateVolume();
                  break;
                  }
            case Channel::Prop::PAN: {
                  updatePan();
                  break;
                  }
            case Channel::Prop::CHORUS: {
                  updateChorus();
                  break;
                  }
            case Channel::Prop::REVERB: {
                  updateReverb();
                  break;
                  }
            case Channel::Prop::NAME: {
                  updateName();
                  break;
                  }
            default:
                  break;
            }

      blockSignals(false);
      }

//MARK:- Methods to update specific elements
// updatePatch - is there a missing case here?- can the patch
// be updated outwith the mixer - and if it is are we listening
// for that change? - not clear that we are

void MixerDetails::updateName()
      {
      channelLabel->setText(selectedMixerTrackItem->getChannelName());
      }


void MixerDetails::updatePatch()
      {
      drumkitCheck->setChecked(selectedMixerTrackItem->getUseDrumset());
      selectedMixerTrackItem->populatePatchCombo(patchCombo);
      }


void MixerDetails::updateVolume()
      {
      volumeSlider->setValue(selectedMixerTrackItem->getVolume());
      volumeSpinBox->setValue(selectedMixerTrackItem->getVolume());
      }

void MixerDetails::updatePan()
      {
      int pan = selectedMixerTrackItem->getPan();
      panSlider->setValue(pan);
      panSpinBox->setValue(pan);
      }


QPushButton* MixerDetails::makeMuteButton(int staff, int voice) {

      QPushButton* muteButton = new QPushButton;
      muteButton->setStyleSheet(QString("QPushButton{padding: 2px 4px 2px 4px;}QPushButton:checked{background-color:%1; color: white;}")
                                .arg(MScore::selectColor[voice].name()));
      muteButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
      muteButton->setMaximumWidth(20);
      muteButton->setMaximumHeight(20);
      muteButton->setText(QString("%1").arg(voice + 1));
      muteButton->setCheckable(true);

      QString helpfulDescription = QString(tr("Mute Voice #%1 on Staff #%2")).arg(voice + 1).arg(staff + 1);
      muteButton->setObjectName(helpfulDescription);
      muteButton->setToolTip(helpfulDescription);
      muteButton->setAccessibleName(helpfulDescription);
      return muteButton;
}

void MixerDetails::updateMutePerVoice()
      {
      for (QWidget* voiceButton : voiceButtons) {
            mutePerVoiceGridLayout->removeWidget(voiceButton);
            voiceButton->deleteLater();
            }

      voiceButtons.clear();

      QList<QList<bool>> mutedStaves = selectedMixerTrackItem->getMutedVoices();

      for (int staffIndex = 0; staffIndex < mutedStaves.length(); ++staffIndex) {
            QList<bool> mutedVoices = mutedStaves[staffIndex];

            if (staffIndex > 0) {
                  mutePerVoiceGridLayout->setColumnMinimumWidth(4, 10);
            }

            for (int voice = 0; voice < mutedVoices.length(); ++voice) {
                  QPushButton* muteButton = makeMuteButton(staffIndex, voice);
                  muteButton->setChecked(mutedVoices[voice]);

                  int column = !(staffIndex % 2) ? voice : voice + 4 + 1;
                  mutePerVoiceGridLayout->addWidget(muteButton, staffIndex / 2, column);

                  MixerVoiceMuteButtonHandler* handler = new MixerVoiceMuteButtonHandler(this, staffIndex, voice, muteButton);
                  connect(muteButton, SIGNAL(toggled(bool)), handler, SLOT(buttonToggled(bool)));
                  voiceButtons.append(muteButton);
                  }
            }
            updateTabOrder();
      }



void MixerDetails::updateMidiChannelAndPort()
      {
      //TODO: midi code moved - needs more testing
      portSpinBox->setValue(selectedMixerTrackItem->getMidiPort());
      channelSpinBox->setValue(selectedMixerTrackItem->getMidiChannel());
//      Part* part = selectedMixerTrackItem->part();
//      Channel* channel = selectedMixerTrackItem->channel();
//      portSpinBox->setValue(part->masterScore()->midiMapping(channel->channel())->port() + 1);
//      channelSpinBox->setValue(part->masterScore()->midiMapping(channel->channel())->channel() + 1);
      }


void MixerDetails::updateReverb()
      {
      reverbSlider->setValue(selectedMixerTrackItem->getReverb());
      reverbSpinBox->setValue(selectedMixerTrackItem->getReverb());
      }


void MixerDetails::updateChorus()
      {
      chorusSlider->setValue(selectedMixerTrackItem->getChorus());
      chorusSpinBox->setValue(selectedMixerTrackItem->getChorus());
      }


//MARK:- Methods to respond to user initiated changes

//  patchChanged - process signal from patchCombo
void MixerDetails::patchComboEdited(int comboIndex)
      {
      if (!selectedMixerTrackItem)
            return;

      mixer->saveTreeSelection();
      selectedMixerTrackItem->changePatch(comboIndex, patchCombo);
      mixer->restoreTreeSelection();
      }

// drumkitToggled - process signal from drumkitCheck
void MixerDetails::drumsetCheckboxToggled(bool useDrumset)
      {
      if (!selectedMixerTrackItem)
            return;

      blockSignals(true);
      mixer->saveTreeSelection();
      selectedMixerTrackItem->setUseDrumset(useDrumset);
      mixer->restoreTreeSelection();
      blockSignals(false);
      }


// volumeChanged - process signal from volumeSlider
void MixerDetails::volumeSpinBoxEdited(int proposedValue)
      {
      if (!selectedMixerTrackItem)
            return;

      int acceptedValue = selectedMixerTrackItem->setVolume(proposedValue);

      if (acceptedValue != proposedValue)
            volumeSpinBox->setValue(acceptedValue);
      }

// volumeChanged - process signal from volumeSpinBox
void MixerDetails::volumeSliderMoved(int proposedValue)
      {
      if (!selectedMixerTrackItem)
            return;
            
      int acceptedValue = selectedMixerTrackItem->setVolume(proposedValue);

      if (acceptedValue != proposedValue)
            volumeSlider->setValue(acceptedValue);
      }


// panChanged - process signal from panSlider
void MixerDetails::panSpinBoxEdited(int proposedValue)
      {
      if (!selectedMixerTrackItem)
                  return;

      int acceptedValue = selectedMixerTrackItem->setPan(proposedValue);

      if (acceptedValue != proposedValue)
            panSpinBox->setValue(acceptedValue);
      }

// panChanged - process signal from panSpinBox
void MixerDetails::panSliderMoved(int proposedValue)
      {
      if (!selectedMixerTrackItem)
            return;

      int acceptedValue = selectedMixerTrackItem->setPan(proposedValue);
            
      if (acceptedValue != proposedValue)
            panSlider->setValue(acceptedValue);
      }


// reverbChanged - process signal from reverbSlider
void MixerDetails::reverbSliderMoved(int proposedValue)
      {
      if (!selectedMixerTrackItem)
            return;

      int acceptedValue = selectedMixerTrackItem->setReverb(proposedValue);

      if (acceptedValue != proposedValue)
            reverbSlider->setValue(acceptedValue);
      }


void MixerDetails::reverbSpinBoxEdited(int proposedValue)
      {
      if (!selectedMixerTrackItem)
            return;

      int acceptedValue = selectedMixerTrackItem->setReverb(proposedValue);

      if (acceptedValue != proposedValue)
            reverbSpinBox->setValue(acceptedValue);
      }


//  chorusChanged - process signal from chorusSlider
void MixerDetails::chorusSliderMoved(int proposedValue)
      {
      if (!selectedMixerTrackItem)
            return;
      int acceptedValue = selectedMixerTrackItem->setChorus(proposedValue);

      if (acceptedValue != proposedValue)
            chorusSlider->setValue(acceptedValue);
      }


void MixerDetails::chorusSpinBoxEdited(int proposedValue)
      {
      if (!selectedMixerTrackItem)
            return;

      int acceptedValue = selectedMixerTrackItem->setChorus(proposedValue);

      if (acceptedValue != proposedValue)
            chorusSpinBox->setValue(acceptedValue);
      }


// voiceMuteButtonToggled - process button toggled (received via MixerVoiceMuteButtonHandler object)
void MixerDetails::voiceMuteButtonToggled(int staffIndex, int voiceIndex, bool shouldMute)
      {
      selectedMixerTrackItem->toggleMutedVoice(staffIndex, voiceIndex, shouldMute);
      }


// midiChannelChanged - process signal from either portSpinBox
// or channelSpinBox, i.e. MIDI port or channel change
void MixerDetails::midiChannelOrPortEdited(int)
      {
      //TODO: midi code moved - needs more testing
      if (!selectedMixerTrackItem)
            return;

//      Part* part = selectedMixerTrackItem->part();
//      Channel* channel = selectedMixerTrackItem->channel();
//
//      seq->stopNotes(channel->channel());
//      int p =    portSpinBox->value() - 1;
//      int c = channelSpinBox->value() - 1;
//
//      MidiMapping* midiMap = selectedMixerTrackItem->midiMap();
//      part->masterScore()->updateMidiMapping(midiMap->articulation(), part, p, c);
//
//      part->score()->setInstrumentsChanged(true);
//      part->score()->setLayoutAll();
//      seq->initInstruments();
//
//      // Update MIDI Out ports
//      int maxPort = max(p, part->score()->masterScore()->midiPortCount());
//      part->score()->masterScore()->setMidiPortCount(maxPort);
//      if (seq->driver() && (preferences.getBool(PREF_IO_JACK_USEJACKMIDI) || preferences.getBool(PREF_IO_ALSA_USEALSAAUDIO)))
//            seq->driver()->updateOutPortCount(maxPort + 1);
      }



//MARK:- Helper methods

// Not 100% sure this is needed. Originally handled voiceMuteButtons separately
// but that's no longer applicable (they work fine WITHIN a group). Do controls
// pop out of widget order if they are not displayed anyway?!
void MixerDetails::updateTabOrder()

      {
      QList<QWidget*> tabOrder = {};

      tabOrder.append({
                            drumkitCheck,
                            patchCombo,
                            volumeSlider, volumeSpinBox,
                            panSlider, panSpinBox
                            });

      tabOrder.append(voiceButtons);

      if (mixer->getOptions()->showMidiOptions())
            tabOrder.append({
                                  portSpinBox, channelSpinBox,
                                  reverbSlider, reverbSpinBox,
                                  chorusSlider, chorusSpinBox});

      QWidget* current = tabOrder.first();
      while (tabOrder.count() > 1) {
            tabOrder.removeFirst();
            QWidget* next = tabOrder.first();
            setTabOrder(current, next);
            current = next;
            }
      }


void MixerDetails::resetControls()
      {
      drumkitCheck->setChecked(false);
      patchCombo->clear();
      channelLabel->setText("");
      volumeSlider->setValue(0);
      volumeSpinBox->setValue(0);
      panSlider->setValue(0);
      panSpinBox->setValue(0);
      reverbSlider->setValue(0);
      reverbSpinBox->setValue(0);
      chorusSlider->setValue(0);
      chorusSpinBox->setValue(0);
      portSpinBox->setValue(0);
      channelSpinBox->setValue(0);
      }


void MixerDetails::blockSignals(bool block)
      {
      volumeSlider->blockSignals(block);
      volumeSpinBox->blockSignals(block);
      panSlider->blockSignals(block);
      panSpinBox->blockSignals(block);
      reverbSlider->blockSignals(block);
      reverbSpinBox->blockSignals(block);
      chorusSlider->blockSignals(block);
      chorusSpinBox->blockSignals(block);
      portSpinBox->blockSignals(block);
      channelSpinBox->blockSignals(block);
      }
}


