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

#include "mixertrackitem.h"

#include "musescore.h"                    // required for access to synti
#include "synthesizer/msynthesizer.h"     // required for MidiPatch
#include "seq.h"

#include "libmscore/score.h"
#include "libmscore/part.h"
#include "libmscore/undo.h"
#include "preferences.h"

#include "mixer.h"
#include "mixeroptions.h"
#include "mixertrackchannel.h"

/*
 A MixerTrackItem object:
 EITHER (1) represents a channel that is one sound source for an instrument that
 in turn belongs to a part. It provides a uniform / clean interface for
 interacting with the sound source in the mixer.

 OR (2) represents a collection of channels that form the variant sound sources for an
 instrument. Implements rules whereby changes to the top level (the collection level)
 are trickled down to the sub-levels (indidvidual channels).
 
 TODO: Clarify my understanding - the enum cases are {PART, CHANNEL}, but, I think, that's
 at odds with how the terminology is used elsewhere. The TrackTypes are, I think, better
 described as:
 - Instrument (one or more channels as a sound source)
 - Channel (a sound source that belongs to an instrument)
 
 The set methods, e.g. setVolume, setReverb etc. apply changes to the underlying channel.
 When thes changes are applied to the underlying channel, any listeners to that channel
 are notified by a propertyChanged() call.
 */

namespace Ms {

//---------------------------------------------------------
//   MixerTrackItem
//---------------------------------------------------------

// General purpose constructor
MixerTrackItem::MixerTrackItem(TrackType trackType, Part* part, Instrument* instr, Channel *chan)
      :_trackType(trackType), _part(part), _instrument(instr), _channel(chan)
      {
      }
      

MixerTrackItem::MixerTrackItem(Part* part, Score* score)
      {
      _trackType = TrackType::PART;
      _part = part;
      _instrument = nullptr;
      _channel = nullptr;
      
      const InstrumentList* instrumenList = part->instruments();

      if (instrumenList->empty())
            return;

      instrumenList->begin();
      _instrument = instrumenList->begin()->second;
      _channel = _instrument->playbackChannel(0, score->masterScore());
      }

//---------------------------------------------------------
//   midiMap
//---------------------------------------------------------

MidiMapping *MixerTrackItem::midiMap()
      {
      return _part->masterScore()->midiMapping(channel()->channel());
      }

//---------------------------------------------------------
//   playbackChannel
//---------------------------------------------------------
//TODO: - suspect this can be eliminated - it's only called in ONE place now
Channel* MixerTrackItem::playbackChannel(const Channel* channel)
      {
      return _part->masterScore()->playbackChannel(channel);
      }

//---------------------------------------------------------
//   color
//---------------------------------------------------------

QString MixerTrackItem::detailedToolTip()
      {

      MidiPatch* midiPatch = synti->getPatchInfo(_channel->synti(), _channel->bank(), _channel->program());

      return QApplication::tr("Part Name: %1\n"
                              "Instrument: %2\n"
                              "Channel: %3\n"
                              "Bank: %4\n"
                              "Program: %5\n"
                              "Patch: %6")
      .arg(_part->partName(),
           _instrument->trackName(),
           qApp->translate("InstrumentsXML", _channel->name().toUtf8().data()),
           QString::number(_channel->bank()),
           QString::number(_channel->program()),
           midiPatch ? midiPatch->name : QApplication::tr("~no patch~"));

      }


QString MixerTrackItem::getName()
      {
      return part()->partName();
      }

QString MixerTrackItem::getChannelName()
      {
      if (channel()->name().isEmpty())
            return "";

      return qApp->translate("InstrumentsXML", channel()->name().toUtf8().data());
      }

void MixerTrackItem::setName(QString newName)
{
      if (part()->partName() == newName) {
            return;
      }

      Score* score = part()->score();
      if (score) {
            score->startCmd();
            score->undo(new ChangePart(part(), part()->instrument(), newName));
            score->endCmd();
      }
}

int MixerTrackItem::color()
      {
      return _trackType ==TrackType::PART ? _part->color() : _channel->color();
      }


char MixerTrackItem::getVolume()
      {
      return channel()->volume();
      }

char MixerTrackItem::getChorus()
{
      return channel()->chorus();
}

char MixerTrackItem::getReverb()
{
      return channel()->reverb();
}

char MixerTrackItem::getPan()
      {
      return channel()->pan() - panAdjustment();
      }

bool MixerTrackItem::getMute()
      {
      return channel()->mute();
      }

bool MixerTrackItem::getSolo()
      {
      return channel()->solo();
      }

// MixerTrackItem settters - when a change is made to underlying channel a propertyChange()
// will be sent to any registered listeners



int MixerTrackItem::setVolume(int proposedValue)
      {

      auto writer = [](int value, Channel* channel){
            channel->setVolume(value);
            seq->setController(channel->channel(), CTRL_VOLUME, channel->volume()); };

      auto reader = [](Channel* channel) -> int {
            return channel->volume(); };

      return adjustValue(proposedValue, reader, writer);
      }


//---------------------------------------------------------
//   setPan
//---------------------------------------------------------
const int MixerTrackItem::panAdjustment() {
      return 63;
}

int MixerTrackItem::setPan(int proposedValue)
      {
      proposedValue = proposedValue + panAdjustment();
      
      auto writer = [](int value, Channel* channel){
            channel->setPan(value);
            seq->setController(channel->channel(), CTRL_PANPOT, channel->pan()); };

      auto reader = [](Channel* channel) -> int {
            return channel->pan(); };

      return adjustValue(proposedValue, reader, writer) - panAdjustment();
      }

//---------------------------------------------------------
//   setChorus
//---------------------------------------------------------

int MixerTrackItem::setChorus(int value)
      {
      auto writer = [](int value, Channel* channel){
            channel->setChorus(value);
            seq->setController(channel->channel(), CTRL_CHORUS_SEND, channel->chorus()); };

      auto reader = [](Channel* channel) -> int {
            return channel->chorus(); };

      return adjustValue(value, reader, writer);
      }

//---------------------------------------------------------
//   setReverb
//---------------------------------------------------------

int MixerTrackItem::setReverb(int value)
      {
      auto writer = [](int value, Channel* channel){
            channel->setReverb(value);
            seq->setController(channel->channel(), CTRL_REVERB_SEND, channel->reverb()); };

      auto reader = [](Channel* channel) -> int {
            return channel->reverb(); };

      return adjustValue(value, reader, writer);
      }


      int MixerTrackItem::getMidiChannel()
      {
      return part()->masterScore()->midiMapping(channel()->channel())->channel() + 1;
      }

      int MixerTrackItem::getMidiPort()
      {
      return part()->masterScore()->midiMapping(channel()->channel())->port() + 1;
      }

void MixerTrackItem::setMidiChannelAndPort(int midiChannel, int midiPort)
      {
      seq->stopNotes(channel()->channel());
      midiPort = midiPort - 1;
      midiChannel = midiChannel - 1;

      part()->masterScore()->updateMidiMapping(midiMap()->articulation(), part(), midiPort, midiChannel);

      part()->score()->setInstrumentsChanged(true);
      part()->score()->setLayoutAll();
      seq->initInstruments();

      // Update MIDI Out ports
      int maxPort = max(midiPort, part()->score()->masterScore()->midiPortCount());
      part()->score()->masterScore()->setMidiPortCount(maxPort);
      if (seq->driver() && (preferences.getBool(PREF_IO_JACK_USEJACKMIDI) || preferences.getBool(PREF_IO_ALSA_USEALSAAUDIO)))
            seq->driver()->updateOutPortCount(maxPort + 1);
}

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void MixerTrackItem::setColor(int valueRgb)
      {
      if (!isPart()) {
            channel()->setColor(valueRgb);
            return;
            }

      // note: does not attempt to respect the relative / override / first channel mode
      _part->setColor(valueRgb);
      for (Channel* channel: playbackChannels()) {
            channel->setColor(valueRgb);
            }
      }

//---------------------------------------------------------
//   setMute
//---------------------------------------------------------

void MixerTrackItem::setMute(bool value)
      {
      if (!isPart()) {
            if (value)
                  seq->stopNotes(_channel->channel());
            channel()->setMute(value);
            return;
            }

      for (Channel* channel: playbackChannels()) {
            if (value)
                  seq->stopNotes(channel->channel());
            channel->setMute(value);
            }
      }

//---------------------------------------------------------
//   setSolo
//---------------------------------------------------------

void MixerTrackItem::setSolo(bool value)
      {

      if (!isPart()) {
            if (value)
                  seq->stopNotes(_channel->channel());
            channel()->setSolo(value);
            }
      else {
            for (Channel* channel: playbackChannels()) {
                  if (value)
                        seq->stopNotes(channel->channel());
                  channel->setSolo(value);
                  }
            }


      //Go through all channels so that all not being soloed get
      // the soloMute property set

      // First, count the number of solo tracks
      int numSolo = 0;
      for (Part* part : _part->score()->parts()) {
            for ( Channel* channel: playbackChannels(part)) {
                  if (channel->solo()) {
                              numSolo++;
                        }
                  }
            }

      // If there are no solo track, clear soloMute in all cases
      // else set soloMute for all non-solo tracks
      for (Part* part : _part->score()->parts()) {
            for ( Channel* channel: playbackChannels(part)) {
                  if (numSolo == 0) {
                        channel->setSoloMute(false);
                        }
                  else {
                        channel->setSoloMute(!channel->solo());
                        if (channel->soloMute())
                              seq->stopNotes(channel->channel());
                        }
                  }
            }
      }

//MARK:- voice muting

void MixerTrackItem::toggleMutedVoice(int staffIndex, int voiceIndex, bool shouldMute)
      {
      Staff* staff = part()->staff(staffIndex);
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

QList<QList<bool>> MixerTrackItem::getMutedVoices()
      {
      QList<QList<bool>> mutedStaves;
      for (int staffIndex = 0; staffIndex < (*part()->staves()).length(); ++staffIndex) {
            Staff* staff = (*part()->staves())[staffIndex];
            QList<bool> mutedVoices;
            for (int voice = 0; voice < VOICES; ++voice) {
                  QString voiceColor = MScore::selectColor[voice].name();
                  bool checked = !staff->playbackVoice(voice);
                  mutedVoices.append(checked);
                  }
      mutedStaves.append(mutedVoices);
      }
      return mutedStaves;
}

//MARK:- helper methods

template <class ChannelWriter, class ChannelReader>
int MixerTrackItem:: adjustValue(int proposedValue, ChannelReader reader, ChannelWriter writer)
{
      if (!isPart()) {
            // only one channel, the easy case - just make a direct adjustment
            writer(proposedValue, _channel);
            return proposedValue;
      }

      // multiple channels and the OVERALL value has been changed
      // make adjustments depending on the OVERALL mode

      MixerOptions::MixerVolumeMode mode = Mixer::getOptions()->mode();

      int currentValue = reader(channel());
      int deltaAdjust = 0;

      switch (mode) {
            case MixerOptions::MixerVolumeMode::PrimaryInstrument:
                  // secondary channels are not touched
                  break;
                  
            case MixerOptions::MixerVolumeMode::Override:
                  for (Channel* channel: secondaryPlaybackChannels()) {
                        // all secondary channels just get newValue
                        writer(proposedValue, channel);
                        }
                  break;
                  
            case MixerOptions::MixerVolumeMode::Ratio:
                  deltaAdjust = relativeAdjust(proposedValue - currentValue, reader, writer);
                  break;
            }

      int acceptedValue = proposedValue + deltaAdjust;

      if (acceptedValue != currentValue)
            writer(acceptedValue, channel());

      return acceptedValue;
}


template <class ChannelWriter, class ChannelReader>
int MixerTrackItem::relativeAdjust(int mainSliderDelta, ChannelReader reader, ChannelWriter writer)
      {
      int lowestValue = 0;
      int highestValue = 0;
      int deltaAdjust = 0;

      // check to see if any channels will go out of bounds
      for (Channel* channel: secondaryPlaybackChannels()) {
            int oldValue = reader(channel);
            int relativeValue = oldValue + mainSliderDelta;
            if (relativeValue < lowestValue)
                  lowestValue = relativeValue;
            if (relativeValue > highestValue)
                  highestValue = relativeValue;
      }

      // if out of bounds, calcualte a delta adjustment to stay within bounds
      if (lowestValue < 0) {
            deltaAdjust = 0 - lowestValue;
      }
      else if (highestValue > 127) {
            deltaAdjust = 127 - highestValue;
      }

      // secondary channels get same increase / decrease as primary value
      if (mainSliderDelta + deltaAdjust != 0) {
            for (Channel* channel: secondaryPlaybackChannels()) {
                  int oldValue = reader(channel);
                  int relativeValue = oldValue + mainSliderDelta + deltaAdjust;
                  writer(max(0, min(127, relativeValue)), channel);
                  }
            }
      return deltaAdjust;
      }




bool MixerTrackItem::isPart()
      {
      return _trackType == TrackType::PART;
      }


//TODO: opportunity for reducing code duplication
//NOTE: the OUTER LOOP:
//      for (Part* p : _part->score()->parts()) {
//            const InstrumentList* il = p->instruments();
//    is used TWICE in the solo and ONCE in updateTracks
//
//    But it uses the same code as in secondaryChannels()
//    So secondaryChannels need to be parameterised to take,
//    say, a part and then return the secondaryChannels for
//    that. Maybe overload so no parameter works just as it
//    is now.

QList<Channel*> MixerTrackItem::secondaryPlaybackChannels() {
      if (!isPart()) {
            return QList<Channel*> {};
      }

      QList<Channel*> allChannels = playbackChannels(_part);
      if (allChannels.isEmpty())
            return allChannels;

      allChannels.removeFirst();
      return allChannels;
}

QList<Channel*> MixerTrackItem::playbackChannels()
      {
      return playbackChannels(_part);
      }


QList<Channel*> MixerTrackItem::playbackChannels(Part* part)
      {
      QList<Channel*> channels;

      //TODO: duplication between here and in Mixer::updateTracks
      // InstrumentList is of the type map<const int, Instrument*>

      const InstrumentList* instrumentList = part->instruments();

      for (auto mapIterator = instrumentList->begin(); mapIterator != instrumentList->end(); ++mapIterator) {
            Instrument* instrument = mapIterator->second;

            for (const Channel* instrumentChannel: instrument->channel()) {
                  Channel* channel = playbackChannel(instrumentChannel);
                  channels.append(channel);
                  }
            }
      return channels;
      }


}

