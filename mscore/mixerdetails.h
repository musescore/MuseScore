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

#ifndef __MIXERDETAILS_H__
#define __MIXERDETAILS_H__

#include "ui_mixerdetails.h"
#include "libmscore/instrument.h"   // needed for ChannelListener
#include "mixertrackitem.h"
#include "mixer.h"



namespace Ms {


class MixerDetails : public QWidget, public Ui::MixerDetails, public ChannelListener
      {
      Q_OBJECT

      Mixer* mixer;
      MixerTrackItem* selectedMixerTrackItem = nullptr;
      void setupSlotsAndSignals();
      QGridLayout* mutePerVoiceGrid;
      QList<QWidget*> voiceButtons; // used for dynamically updating tabOrder

      void updateName();
      void updatePatch();
      void updateVolume();
      void updatePan();
      void updateMutePerVoice();
      QPushButton* makeMuteButton(int staff, int voice);
      void updateMidiChannelAndPort();
      void updateReverb();
      void updateChorus();

      void blockSignals(bool);
      void updateTabOrder();
            
   public slots:
      void drumsetCheckboxToggled(bool);
      void patchComboEdited(int);
      void volumeSliderMoved(int);
      void volumeSpinBoxEdited(int);
      void panSliderMoved(int);
      void panSpinBoxEdited(int);
      void midiChannelOrPortEdited(int);
      void reverbSliderMoved(int);
      void reverbSpinBoxEdited(int);
      void chorusSliderMoved(int);
      void chorusSpinBoxEdited(int);
      void updateDetails(MixerTrackItem*);

   public:
      MixerDetails(Mixer *mixer);
      void propertyChanged(Channel::Prop property) override;

      void resetControls(); // apply default (0 or empty) values for when no track is selected
      void voiceMuteButtonToggled(int staffIndex, int voiceIndex, bool shouldMute);
      void updateUiOptions();

      MixerTrackItem* getSelectedMixerTrackItem() { return selectedMixerTrackItem; };
      };


class MixerDetails;

class MixerVoiceMuteButtonHandler : public QObject
      {
      Q_OBJECT

      MixerDetails* mixerDetails;
      int staffIndex;
      int voiceIndex;

   public:
      MixerVoiceMuteButtonHandler(MixerDetails* mixerDetails, int staffIndex, int voiceIndex, QObject* parent = nullptr)
            : QObject(parent),
              mixerDetails(mixerDetails),
              staffIndex(staffIndex),
              voiceIndex(voiceIndex)
            {}

public slots:
      void buttonToggled(bool checked)
            {
            mixerDetails->voiceMuteButtonToggled(staffIndex, voiceIndex, checked);
            }
   };
}
#endif // __MIXERDETAILS_H__
