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

#include "mixertrackchannel.h"

#include "libmscore/instrument.h"
#include "musescore.h"
#include "synthesizer/msynthesizer.h"     // required for MidiPatch

#include "mixer.h"
#include "mixertrackitem.h"
#include "mixeroptions.h"

namespace Ms {

//--------------------------------------------------------------
//  MixerTrackChannel provides an widget that is displayed in a
//  row of a QTreeWidget. The widget includes a slider (by default
//  to control track volume) and Mute and Solo Buttons. The widget
//  is a "listener" to the track that is controls. This means that
//  when other parts of MuseScore change the track, the control
//  will update itself.
//--------------------------------------------------------------
MixerTrackChannel::MixerTrackChannel(MixerTreeWidgetItem* treeWidgetItem) :
      treeWidgetItem(treeWidgetItem)
      {
      setupUi(this);
      setupAdditionalUi();
      updateUiControls();
      setupSlotsAndSignals();
      update();

      Channel* channel = mixerTrackItem()->channel();
      channel->addListener(this);
      }



void MixerTrackChannel::setupSlotsAndSignals()
      {
      connect(muteButton,     SIGNAL(toggled(bool)),        SLOT(stripMuteToggled(bool)));
      connect(soloButton,     SIGNAL(toggled(bool)),        SLOT(stripSoloToggled(bool)));
      connect(trackSlider,   SIGNAL(valueChanged(int)),    SLOT(stripVolumeSliderMoved(int)));
      connect(trackSlider,   SIGNAL(sliderPressed()),      SLOT(takeSelection()));
      }

void MixerTrackChannel::takeSelection()
      {
      treeWidgetItem->treeWidget()->setCurrentItem(treeWidgetItem);
      }

void MixerTrackChannel::setupAdditionalUi()
      {
      //TODO: a more responsible approach to styling that's also light/dark theme respectful
      QString basicButton = "QToolButton{background: white; color: black; font-weight: bold; border: 1px solid gray;}";
      QString colorTemplate = "QToolButton:checked, QToolButton:pressed { color: white; background: %1;}";
      muteButton->setStyleSheet(basicButton + colorTemplate.arg("red"));
      soloButton->setStyleSheet(basicButton + colorTemplate.arg("green"));
      }

void MixerTrackChannel::updateUiControls()
      {
      bool showTrackColors = Mixer::getOptions()->showTrackColors();
      colorLabel->setVisible(showTrackColors);
      bool secondaryMode = Mixer::getOptions()->secondaryModeOn();
      trackSlider->setSecondaryMode(secondaryMode);
      trackSlider->blockSignals(true);
      trackSlider->setPanMode(secondaryMode && Mixer::getOptions()->secondarySlider() == MixerOptions::MixerSecondarySlider::Pan);
      update();
      trackSlider->blockSignals(false);
      }

void MixerTrackChannel::update()
      {
      const QSignalBlocker blockVolumeSignals(trackSlider);
      const QSignalBlocker blockMuteSignals(muteButton);
      const QSignalBlocker blockSoloSignals(soloButton);

      MixerOptions* options = Mixer::getOptions();

      int value;
      QString tooltip;

      if (options->secondaryModeOn()) {
            switch (options->secondarySlider()) {
                  case MixerOptions::MixerSecondarySlider::Pan:
                        value = mixerTrackItem()->getPan();
                        tooltip = tr("Pan: %1");
                        break;
                  case MixerOptions::MixerSecondarySlider::Reverb:
                        value = mixerTrackItem()->getReverb();
                        tooltip = tr("Reverb: %1");
                        break;
                  case MixerOptions::MixerSecondarySlider::Chorus:
                        value = mixerTrackItem()->getChorus();
                        tooltip = tr("Chorus: %1");
                        break;
                  }
            }
      else {
            value = mixerTrackItem()->getVolume();
            tooltip = tr("Volume: %1");
            }


      trackSlider->setValue(value);
      trackSlider->setToolTip(tooltip.arg(QString::number(value)));
      
      muteButton->setChecked(mixerTrackItem()->getMute());
      soloButton->setChecked(mixerTrackItem()->getSolo());

      QColor channelColor = mixerTrackItem()->color();
      if (colorLabel)
            colorLabel->setStyleSheet(QString("QLabel{background: %1;padding-top: 2px; padding-bottom: 2px; border-radius: 3px;}").arg(channelColor.name()));

      setToolTip(mixerTrackItem()->detailedToolTip());
      }


void MixerTrackChannel::propertyChanged(Channel::Prop property)
      {
      update();
      }


void MixerTrackChannel::stripVolumeSliderMoved(int proposedValue)
      {
      takeSelection();
      MixerOptions* options = Mixer::getOptions();

      int acceptedValue;

      if (options->secondaryModeOn()) {
            switch (options->secondarySlider()) {
                  case MixerOptions::MixerSecondarySlider::Pan:
                        acceptedValue = mixerTrackItem()->setPan(proposedValue);
                        break;
                  case MixerOptions::MixerSecondarySlider::Reverb:
                        acceptedValue = mixerTrackItem()->setReverb(proposedValue);
                        break;
                  case MixerOptions::MixerSecondarySlider::Chorus:
                        acceptedValue = mixerTrackItem()->setChorus(proposedValue);
                        break;
            }
      }
      else {
            acceptedValue = mixerTrackItem()->setVolume(proposedValue);
      }

      if (acceptedValue != proposedValue)
      trackSlider->setValue(acceptedValue);
      }


void MixerTrackChannel::stripSoloToggled(bool val)
      {
      mixerTrackItem()->setSolo(val);
      takeSelection();
      }


void MixerTrackChannel::stripMuteToggled(bool val)
      {
      mixerTrackItem()->setMute(val);
      takeSelection();
      }

}
