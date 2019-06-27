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

#include "musescore.h"

#include "libmscore/score.h"
#include "libmscore/part.h"
#include "mixertrack.h"
#include "mixertrackitem.h"
#include "seq.h"
#include "libmscore/undo.h"
#include "synthcontrol.h"
#include "synthesizer/msynthesizer.h"
#include "preferences.h"

namespace Ms {

//MARK:- Create and setup
MixerDetails::MixerDetails(Mixer *mixer) :
      QWidget(mixer), mixer(mixer),
      selectedMixerTrackItem(nullptr)
      {
      setupUi(this);

      mutePerVoiceGrid = new QGridLayout();
      mutePerVoiceHolder->setLayout(mutePerVoiceGrid);
      mutePerVoiceGrid->setContentsMargins(0, 0, 0, 0);
      mutePerVoiceGrid->setSpacing(7);

      updateUiOptions();                        // show or hide certain controls as per user preferences
      setupSlotsAndSignals();
      updateDetails(selectedMixerTrackItem);    // when called with nullptr will reset and disable all controls
      }


void MixerDetails::setupSlotsAndSignals()
      {
      connect(partNameLineEdit,     SIGNAL(editingFinished()),    SLOT(partNameEdited()));
      connect(trackColorLabel,      SIGNAL(colorChanged(QColor)), SLOT(trackColorEdited(QColor)));
      connect(drumkitCheck,         SIGNAL(toggled(bool)),        SLOT(drumsetCheckboxToggled(bool)));
      connect(patchCombo,           SIGNAL(activated(int)),       SLOT(patchComboEdited(int)));
      connect(volumeSlider,         SIGNAL(valueChanged(int)),    SLOT(volumeSliderMoved(int)));
      connect(volumeSpinBox,        SIGNAL(valueChanged(double)), SLOT(volumeSpinBoxEdited(double)));
      connect(panSlider,            SIGNAL(valueChanged(int)),    SLOT(panSliderMoved(int)));
      connect(panSpinBox,           SIGNAL(valueChanged(double)), SLOT(panSpinBoxEdited(double)));
      connect(portSpinBox,          SIGNAL(valueChanged(int)),    SLOT(midiChannelOrPortEdited(int)));
      connect(channelSpinBox,       SIGNAL(valueChanged(int)),    SLOT(midiChannelOrPortEdited(int)));
      connect(chorusSlider,         SIGNAL(valueChanged(int)),    SLOT(chorusSliderMoved(int)));
      connect(chorusSpinBox,        SIGNAL(valueChanged(double)), SLOT(chorusSpinBoxEdited(double)));
      connect(reverbSlider,         SIGNAL(valueChanged(int)),    SLOT(reverbSliderMoved(int)));
      connect(reverbSpinBox,        SIGNAL(valueChanged(double)), SLOT(reverbSpinBoxEdited(double)));
      }

void MixerDetails::updateUiOptions()
      {
      MixerOptions* options = Mixer::getOptions();
      bool showTrackColors = options->showTrackColors();
      trackColorLabel->setVisible(showTrackColors);
      labelTrackColor->setVisible(showTrackColors);

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
      updateTrackColor();
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
            case Channel::Prop::COLOR: {
                  updateTrackColor();
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
      Part* part = selectedMixerTrackItem->part();
      Channel* channel = selectedMixerTrackItem->channel();
      QString partName = part->partName();
      if (!channel->name().isEmpty())
            channelLabel->setText(qApp->translate("InstrumentsXML", channel->name().toUtf8().data()));
      else
            channelLabel->setText("");
      partNameLineEdit->setText(partName);
      partNameLineEdit->setToolTip(partName);
      }


void MixerDetails::updateTrackColor()
      {
      trackColorLabel->setColor(QColor(selectedMixerTrackItem->color() | 0xff000000));
      }


void MixerDetails::updatePatch()
      {
      Channel* channel = selectedMixerTrackItem->channel();
      MidiMapping* midiMap = selectedMixerTrackItem->midiMap();
      
      //Check if drumkit
      const bool drum = midiMap->part()->instrument()->useDrumset();
      drumkitCheck->setChecked(drum);
      
      //Populate patch combo
      patchCombo->clear();
      const auto& pl = synti->getPatchInfo();
      int patchIndex = 0;


      // Order by program number instead of bank, so similar instruments
      // appear next to each other, but ordered primarily by soundfont
      std::map<int, std::map<int, std::vector<const MidiPatch*>>> orderedPl;
      
      for (const MidiPatch* p : pl)
            orderedPl[p->sfid][p->prog].push_back(p);
      
      std::vector<QString> usedNames;
      for (auto const& sf : orderedPl) {
            for (auto const& pn : sf.second) {
                  for (const MidiPatch* p : pn.second) {
                        if (p->drum == drum || p->synti != "Fluid") {
                              QString pName = p->name;
                              if (std::find(usedNames.begin(), usedNames.end(), p->name) != usedNames.end()) {
                                    QString addNum = QString(" (%1)").arg(p->sfid);
                                    pName.append(addNum);
                                    }
                              else
                                    usedNames.push_back(p->name);
                              
                              patchCombo->addItem(pName, QVariant::fromValue<void*>((void*)p));
                              if (p->synti == channel->synti() &&
                                  p->bank == channel->bank() &&
                                  p->prog == channel->program())
                                    patchIndex = patchCombo->count() - 1;
                              }
                        }
                  }
            }
      patchCombo->setCurrentIndex(patchIndex);
      }




void MixerDetails::updateVolume()
      {
      Channel* channel = selectedMixerTrackItem->channel();
      volumeSlider->setValue((int)channel->volume());
      volumeSpinBox->setValue(channel->volume());
      }

void MixerDetails::updatePan()
      {
      int pan = selectedMixerTrackItem->getPan()-63;
      panSlider->setValue(pan);
      //panSlider->setToolTip(tr("Pan: %1").arg(QString::number(pan)));
      panSpinBox->setValue(pan);
      }

void MixerDetails::updateMutePerVoice()
      {
      qDebug()<<"MixerDetails::updateMutePerVoice - could I do some caching? checking for change?";

      for (QWidget* voiceButton : voiceButtons) {
            mutePerVoiceGridLayout->removeWidget(voiceButton);
            voiceButton->deleteLater();
            }

      voiceButtons.clear();

      Part* part = selectedMixerTrackItem->part();

      for (int staffIndex = 0; staffIndex < (*part->staves()).length(); ++staffIndex) {
            Staff* staff = (*part->staves())[staffIndex];
            for (int voice = 0; voice < VOICES; ++voice) {
                  QPushButton* muteButton = new QPushButton;
                  muteButton->setStyleSheet(
                                    QString("QPushButton{padding: 4px 8px 4px 8px;}QPushButton:checked{background-color:%1}")
                                    .arg(MScore::selectColor[voice].name()));
                  muteButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
                  muteButton->setMaximumWidth(30);
                  muteButton->setText(QString("%1").arg(voice + 1));
                  muteButton->setCheckable(true);
                  muteButton->setChecked(!staff->playbackVoice(voice));
                  QString helpfulDescription = QString(tr("Mute Voice #%1 on Staff #%2")).arg(voice + 1).arg(staffIndex + 1);
                  muteButton->setObjectName(helpfulDescription);
                  muteButton->setToolTip(helpfulDescription);
                  muteButton->setAccessibleName(helpfulDescription);

                  mutePerVoiceGridLayout->addWidget(muteButton, staffIndex, voice);
                  MixerVoiceMuteButtonHandler* handler = new MixerVoiceMuteButtonHandler(this, staffIndex, voice, muteButton);
                  connect(muteButton, SIGNAL(toggled(bool)), handler, SLOT(buttonToggled(bool)));
                  voiceButtons.append(muteButton);
                  }
            }
      }

void MixerDetails::updateMidiChannelAndPort()
      {
      Part* part = selectedMixerTrackItem->part();
      Channel* channel = selectedMixerTrackItem->channel();
      portSpinBox->setValue(part->masterScore()->midiMapping(channel->channel())->port() + 1);
      channelSpinBox->setValue(part->masterScore()->midiMapping(channel->channel())->channel() + 1);
      }

void MixerDetails::updateReverb()
      {
      Channel* channel = selectedMixerTrackItem->channel();
      reverbSlider->setValue((int)channel->reverb());
      reverbSpinBox->setValue(channel->reverb());
      }


void MixerDetails::updateChorus()
      {
      Channel* channel = selectedMixerTrackItem->channel();
      reverbSlider->setValue((int)channel->reverb());
      reverbSpinBox->setValue(channel->reverb());
      }


//MARK:- Methods to respond to user initiated changes

// partNameEdited - process editing complete on part name
void MixerDetails::partNameEdited()
      {
      qDebug()<<"MixerDetails::partNameEdited";
      if (!selectedMixerTrackItem)
            return;

      QString text = partNameLineEdit->text();
      Part* part = selectedMixerTrackItem->part();
      if (part->partName() == text) {
            return;
            }

      mixer->saveTreeSelection();
      Score* score = part->score();
      if (score) {
            score->startCmd();
            score->undo(new ChangePart(part, part->instrument(), text));
            score->endCmd();
            }
      mixer->restoreTreeSelection();
      }

// trackColorEdited
void MixerDetails::trackColorEdited(QColor col)
      {
      if (!selectedMixerTrackItem)
            return;

      selectedMixerTrackItem->setColor(col.rgb());
      }

//  patchChanged - process signal from patchCombo
void MixerDetails::patchComboEdited(int comboIndex)
      {
      if (!selectedMixerTrackItem)
            return;

      const MidiPatch* patch = (MidiPatch*)patchCombo->itemData(comboIndex, Qt::UserRole).value<void*>();
      if (patch == 0) {
            qDebug("PartEdit::patchChanged: no patch");
            return;
            }

      Part* part = selectedMixerTrackItem->midiMap()->part();
      Channel* channel = selectedMixerTrackItem->midiMap()->articulation();

      mixer->saveTreeSelection();

      Score* score = part->score();
      if (score) {
            score->startCmd();
            score->undo(new ChangePatch(score, channel, patch));
            score->undo(new SetUserBankController(channel, true));
            score->setLayoutAll();
            score->endCmd();
            }

      mixer->restoreTreeSelection();
      }

// drumkitToggled - process signal from drumkitCheck
void MixerDetails::drumsetCheckboxToggled(bool drumsetSelected)
      {
      if (!selectedMixerTrackItem)
            return;

      blockSignals(true);

      qDebug()<<"drumsetCheckBoxToggled to: "<<drumsetSelected;

      Part* part = selectedMixerTrackItem->part();
      Channel* channel = selectedMixerTrackItem->channel();

      Instrument *instr;
      if (selectedMixerTrackItem->trackType() == MixerTrackItem::TrackType::CHANNEL)
            instr = selectedMixerTrackItem->instrument();
      else
            instr = part->instrument(Fraction(0,1));

      qDebug()<<"drumsetCheckBoxToggled - trackType==CHANNEL is "<<(selectedMixerTrackItem->trackType() == MixerTrackItem::TrackType::CHANNEL);


      if (instr->useDrumset() == drumsetSelected)
            return;

      const MidiPatch* newPatch = 0;
      const QList<MidiPatch*> pl = synti->getPatchInfo();
      for (const MidiPatch* p : pl) {
            if (p->drum == drumsetSelected) {
                  newPatch = p;
                  break;
                  }
            }

      qDebug()<<"drumsetCheckBoxToggled - candidate patch is"<<newPatch;
      if (newPatch) {
            QString name = newPatch->name;
            qDebug()<<"drumsetCheckBoxToggled - candidate patch is called: "<<name;
            }


      mixer->saveTreeSelection();

      qDebug()<<"drumsetCheckBoxToggled -  saved selection now trying to change patch";

      Score* score = part->score();
      if (newPatch) {
            score->startCmd();
            part->undoChangeProperty(Pid::USE_DRUMSET, drumsetSelected);
            score->undo(new ChangePatch(score, channel, newPatch));
            score->setLayoutAll();
            score->endCmd();
            }

      qDebug()<<"drumsetCheckBoxToggled - change patch now trying to restore selection";

      mixer->restoreTreeSelection();
      blockSignals(false);
      }


// volumeChanged - process signal from volumeSlider
void MixerDetails::volumeSpinBoxEdited(double value)
      {
      if (!selectedMixerTrackItem)
            return;
      selectedMixerTrackItem->setVolume(value);
      }

// volumeChanged - process signal from volumeSpinBox
void MixerDetails::volumeSliderMoved(int value)
      {
      if (!selectedMixerTrackItem)
            return;
      selectedMixerTrackItem->setVolume(value);
      }


// panChanged - process signal from panSlider
void MixerDetails::panSpinBoxEdited(double value)
      {
      panSliderMoved(int(value));
      }

// panChanged - process signal from panSpinBox
void MixerDetails::panSliderMoved(int value)
      {
      // is this required? if mixerDetails is disabled can this ever be called
      if (!selectedMixerTrackItem)
            return;
      // note: a guaranteed side effect is that propertyChanged() will
      // be called on this object - I think that's true?!
      selectedMixerTrackItem->setPan(value + 63);
      }

void MixerDetails::resetPanToCentre()
      {
      panSliderMoved(0);
      }


// voiceMuteButtonToggled - process button toggled (received via MixerVoiceMuteButtonHandler object)
void MixerDetails::voiceMuteButtonToggled(int staffIndex, int voiceIndex, bool shouldMute)
      {
      Part* part = selectedMixerTrackItem->part();
      Staff* staff = part->staff(staffIndex);
      switch (voiceIndex) {
            case 0:
                  staff->undoChangeProperty(Pid::PLAYBACK_VOICE1, !shouldMute);
                  break;
            case 1:
                  staff->undoChangeProperty(Pid::PLAYBACK_VOICE2, !shouldMute);
                  break;
            case 2:
                  staff->undoChangeProperty(Pid::PLAYBACK_VOICE3, !shouldMute);
                  break;
            case 3:
                  staff->undoChangeProperty(Pid::PLAYBACK_VOICE4, !shouldMute);
                  break;
            }
      }


// reverbChanged - process signal from reverbSlider
void MixerDetails::reverbSliderMoved(int value)
      {
      if (!selectedMixerTrackItem)
            return;
      selectedMixerTrackItem->setReverb(value);
      }


void MixerDetails::reverbSpinBoxEdited(double value)
      {
      reverbSliderMoved(int(value));
      }


//  chorusChanged - process signal from chorusSlider
void MixerDetails::chorusSliderMoved(int value)
      {
      if (!selectedMixerTrackItem)
            return;
      selectedMixerTrackItem->setChorus(value);
      }


void MixerDetails::chorusSpinBoxEdited(double value)
      {
      chorusSliderMoved(int(value));
      }


// midiChannelChanged - process signal from either portSpinBox
// or channelSpinBox, i.e. MIDI port or channel change
void MixerDetails::midiChannelOrPortEdited(int)
      {
      if (!selectedMixerTrackItem)
            return;

      Part* part = selectedMixerTrackItem->part();
      Channel* channel = selectedMixerTrackItem->channel();

      seq->stopNotes(channel->channel());
      int p =    portSpinBox->value() - 1;
      int c = channelSpinBox->value() - 1;

      MidiMapping* midiMap = selectedMixerTrackItem->midiMap();
      part->masterScore()->updateMidiMapping(midiMap->articulation(), part, p, c);

      part->score()->setInstrumentsChanged(true);
      part->score()->setLayoutAll();
      seq->initInstruments();

      // Update MIDI Out ports
      int maxPort = max(p, part->score()->masterScore()->midiPortCount());
      part->score()->masterScore()->setMidiPortCount(maxPort);
      if (seq->driver() && (preferences.getBool(PREF_IO_JACK_USEJACKMIDI) || preferences.getBool(PREF_IO_ALSA_USEALSAAUDIO)))
            seq->driver()->updateOutPortCount(maxPort + 1);
      }

//MARK:- Helper methods


// Not 100% sure this is needed. Originally handled voiceMuteButtons separately
// but that's no longer applicable (they work fine WITHIN a group). Do controls
// pop out of wigdet order if they are not displayed anyway?!
void MixerDetails::updateTabOrder()

      {
      QList<QWidget*> tabOrder = {partNameLineEdit};

      if (mixer->getOptions()->showTrackColors())
            tabOrder.append(trackColorLabel);

      tabOrder.append({
                            drumkitCheck,
                            patchCombo,
                            volumeSlider, volumeSpinBox,
                            panSlider, panSpinBox,
                            mutePerVoiceHolder});

      if (mixer->getOptions()->showMidiOptions())
            tabOrder.append({
                                  portSpinBox, channelSpinBox,
                                  reverbSlider, reverbSpinBox,
                                  chorusSlider, chorusSpinBox});

      QWidget* current = tabOrder.first();
      while (tabOrder.count() > 1) {
            tabOrder.removeFirst();
            QWidget* next = tabOrder.first();
            // qDebug()<<"Setting tab order. "<<current->objectName()<<" before "<<next->objectName();
            setTabOrder(current, next);
            current = next;
            }
      }


void MixerDetails::resetControls()
      {
      partNameLineEdit->setText("");
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
      trackColorLabel->setColor(QColor());
      }


void MixerDetails::blockSignals(bool block)
      {
      partNameLineEdit->blockSignals(block);
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
      trackColorLabel->blockSignals(block);
      }
}


