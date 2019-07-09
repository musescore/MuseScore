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
#include <QComboBox>

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


//MARK:- patch

bool  MixerTrackItem::getUseDrumset() {

      //Check if drumkit
      return midiMap()->part()->instrument()->useDrumset();
}

bool MixerTrackItem::isCurrentPatch(const MidiPatch* patch)
      {
         return patch->synti == channel()->synti() &&
            patch->bank == channel()->bank() &&
            patch->prog == channel()->program();
      }

QString MixerTrackItem::adjustedPatchName(const MidiPatch* patch, std::vector<QString> usedNames)
      {
      QString patchName = patch->name;

      if (std::find(usedNames.begin(), usedNames.end(), patchName) != usedNames.end()) {
            QString addNum = QString(" (%1)").arg(patch->sfid);
            patchName.append(addNum);
            }
      else {
            usedNames.push_back(patch->name);
            }

            bool verbose = false;

      if (verbose) {
            return patchName.append(QString(" %1 %2; Bank: %3; Prog: %4; SF: %5")
                  .arg(patch->synti)
                  .arg(patch->drum ? "🥁" : "🎶")
                  .arg(patch->bank)
                  .arg(patch->prog)
                  .arg(patch->sfid));
            }

      return patchName;
      }


void MixerTrackItem::populatePatchCombo(QComboBox* patchCombo)
{
      //Check if drumkit
      bool drum = getUseDrumset();

      //Populate patch combo
      patchCombo->clear();

      const QList<MidiPatch*>& pl = synti->getPatchInfo();
      int patchIndex = 0;

      // Order by program number instead of bank, so similar instruments
      // appear next to each other, but ordered primarily by soundfont
      std::map<int, std::map<int, std::vector<const MidiPatch*>>> orderedPatchList;

      for (const MidiPatch* patch : pl)
            orderedPatchList[patch->sfid][patch->prog].push_back(patch);

      std::vector<QString> usedNames;
      for (auto const& soundfont : orderedPatchList) {
            for (auto const& pn : soundfont.second) {
                  for (const MidiPatch* patch : pn.second) {
                        if (patch->drum == drum || patch->synti != "Fluid") {
                              QString patchName = adjustedPatchName(patch, usedNames);
                              patchCombo->addItem(patchName, QVariant::fromValue<void*>((void*)patch));
                              if (isCurrentPatch(patch))
                                    patchIndex = patchCombo->count() - 1;
                              }
                        }
                  }
            }
      patchCombo->setCurrentIndex(patchIndex);
}


void MixerTrackItem::changePatch(int itemIndex, QComboBox* patchCombo)
{
      const MidiPatch* patch = (MidiPatch*)patchCombo->itemData(itemIndex, Qt::UserRole).value<void*>();

      if (patch == 0) {
            qDebug("MixerTrackItem::patchChanged: no patch");
            return;
      }

      Part* part = midiMap()->part();
      Channel* channel = midiMap()->articulation();

      Score* score = part->score();
      if (score) {
            score->startCmd();
            score->undo(new ChangePatch(score, channel, patch));
            score->undo(new SetUserBankController(channel, true));
            score->setLayoutAll();
            score->endCmd();
      }
}


void MixerTrackItem::setUseDrumset(bool useDrumset)
      {
      Instrument* instr = isPart() ? part()->instrument(Fraction(0,1)) : instrument();

      if (instr->useDrumset() == useDrumset)
            return;

      const MidiPatch* newPatch = 0;
      const QList<MidiPatch*> patchList = synti->getPatchInfo();
      for (const MidiPatch* patch : patchList) {
            if (patch->drum == useDrumset) {
                  newPatch = patch;
                  break;
                  }
            }

      if (newPatch)
            QString name = newPatch->name;

      Score* score = part()->score();
      if (newPatch) {
            score->startCmd();
            part()->undoChangeProperty(Pid::USE_DRUMSET, useDrumset);
            score->undo(new ChangePatch(score, channel(), newPatch));
            score->setLayoutAll();
            score->endCmd();
            }
      }


//MARK:- part and channel name
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
      return isPart() ? _part->color() : _channel->color();
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
      return channel()->pan() - panAdjustment;
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

int MixerTrackItem::setVolume(int proposedValue, bool forceOverride)
      {
      auto writer = [](int value, Channel* channel){
            channel->setVolume(value);
            seq->setController(channel->channel(), CTRL_VOLUME, channel->volume()); };

      auto reader = [](Channel* channel) -> int {
            return channel->volume(); };

      return adjustValue(proposedValue, reader, writer, forceOverride);
      }



int MixerTrackItem::setPan(int proposedValue, bool forceOverride)
      {
      proposedValue = proposedValue + panAdjustment;
      
      auto writer = [](int value, Channel* channel){
            channel->setPan(value);
            seq->setController(channel->channel(), CTRL_PANPOT, channel->pan()); };

      auto reader = [](Channel* channel) -> int {
            return channel->pan(); };

      return adjustValue(proposedValue, reader, writer, forceOverride) - panAdjustment;
      }


int MixerTrackItem::setChorus(int value, bool forceOverride)
      {
      auto writer = [](int value, Channel* channel){
            channel->setChorus(value);
            seq->setController(channel->channel(), CTRL_CHORUS_SEND, channel->chorus()); };

      auto reader = [](Channel* channel) -> int {
            return channel->chorus(); };

      return adjustValue(value, reader, writer, forceOverride);
      }


int MixerTrackItem::setReverb(int value, bool forceOverride)
      {
      auto writer = [](int value, Channel* channel){
            channel->setReverb(value);
            seq->setController(channel->channel(), CTRL_REVERB_SEND, channel->reverb()); };

      auto reader = [](Channel* channel) -> int {
            return channel->reverb(); };

      return adjustValue(value, reader, writer, forceOverride);
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



void MixerTrackItem::setMute(bool muteOn)
      {
      if (!isPart()) {
            if (muteOn)
                  seq->stopNotes(_channel->channel());
            channel()->setMute(muteOn);
            return;
            }

      for (Channel* channel: playbackChannels()) {
            if (muteOn)
                  seq->stopNotes(channel->channel());
            channel->setMute(muteOn);
            }
      }



void MixerTrackItem::setSolo(bool soloOn)
      {
      if (!isPart()) {
            if (soloOn)
                  seq->stopNotes(_channel->channel());
            channel()->setSolo(soloOn);
            }
      else {
            for (Channel* channel: playbackChannels()) {
                  if (soloOn)
                        seq->stopNotes(channel->channel());
                  channel->setSolo(soloOn);
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

      // If there are no soloed tracks, clear soloMute in all cases
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

      
//MARK:: - reset
      
void MixerTrackItem::resetWithVolume(int volume)
      {
      setVolume(volume, true);
      setPan(0, true);
      setChorus(0, true);
      setReverb(0, true);
      setSolo(false);
      setMute(false);
      }



bool  MixerTrackItem::isResetWithVolume(int volume)
{
      if (getVolume() != volume)
            return false;

      if (getPan() != 0)
            return false;

      if (getChorus() != 0)
            return false;

      if (getReverb() != 0)
            return false;

      if (getSolo() != false)
            return false;

      if (getMute() != false)
            return false;

      return true;
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
                  bool checked = !staff->playbackVoice(voice);
                  mutedVoices.append(checked);
                  }
      mutedStaves.append(mutedVoices);
      }
      return mutedStaves;
}

//MARK:- helper methods

template <class ChannelWriter, class ChannelReader>
int MixerTrackItem:: adjustValue(int proposedValue, ChannelReader reader, ChannelWriter writer, bool forceOverride)
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

      if (mode == MixerOptions::MixerVolumeMode::Override || forceOverride) {
            for (Channel* channel: secondaryPlaybackChannels()) {
                  // all secondary channels just get newValue
                  writer(proposedValue, channel);
            }
      }
      else if (mode == MixerOptions::MixerVolumeMode::Ratio) {
            deltaAdjust = relativeAdjust(proposedValue - currentValue, reader, writer);
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

