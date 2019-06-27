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
#include "mixer.h"
#include "libmscore/score.h"
#include "libmscore/part.h"
#include "seq.h"

/* refactoring using closures to reduce the high level of code duplication here */

/* I THINK this could be / should be a sub class of TreeWidgetItem - that's a model
 class and so is - not clear they need to be separated. In addition, this class can
 almost certainly generate children automatically. This will help with updateTracks.

 Still not very clear on how make EXPAND work. The old approach has a SET of expanded
 parts. Need to undertand what happens to items in that set when, say, instruments are
 deleted. Do they stay in the set. And whatever is in the set, are they preserved when
 the user moves from score to score... Don't yet understand the data structures at play
 here.

 */


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

int MixerTrackItem::color()
      {
      return _trackType ==TrackType::PART ? _part->color() : _channel->color();
      }


char MixerTrackItem::getVolume()
      {
      return channel()->volume();
      }

char MixerTrackItem::getPan()
      {
      return channel()->pan();
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



void MixerTrackItem::setVolume(char value)
      {

      auto writer = [](int value, Channel* channel){
            channel->setVolume(value);
            seq->setController(channel->channel(), CTRL_VOLUME, channel->volume()); };

      auto reader = [](Channel* channel) -> int {
            return channel->volume(); };

      adjustValue(value, reader, writer);
      }


//---------------------------------------------------------
//   setPan
//---------------------------------------------------------

void MixerTrackItem::setPan(char value)
      {
      auto writer = [](int value, Channel* channel){
            channel->setPan(value);
            seq->setController(channel->channel(), CTRL_PANPOT, channel->pan()); };

      auto reader = [](Channel* channel) -> int {
            return channel->pan(); };

      adjustValue(value, reader, writer);
      }

//---------------------------------------------------------
//   setChorus
//---------------------------------------------------------

void MixerTrackItem::setChorus(char value)
      {
      auto writer = [](int value, Channel* channel){
            channel->setChorus(value);
            seq->setController(channel->channel(), CTRL_CHORUS_SEND, channel->chorus()); };

      auto reader = [](Channel* channel) -> int {
            return channel->chorus(); };

      adjustValue(value, reader, writer);
      }

//---------------------------------------------------------
//   setReverb
//---------------------------------------------------------

void MixerTrackItem::setReverb(char value)
      {
      auto writer = [](int value, Channel* channel){
            channel->setReverb(value);
            seq->setController(channel->channel(), CTRL_REVERB_SEND, channel->reverb()); };

      auto reader = [](Channel* channel) -> int {
            return channel->reverb(); };

      adjustValue(value, reader, writer);
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

      // TODO: setColor - does not respect the "overall" policy - maybe it should
      _part->setColor(valueRgb); //TODO: - not sure this is necessary (or does anything?!)
      for (Channel* channel: secondaryPlaybackChannels()) {
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

      for (Channel* channel: secondaryPlaybackChannels()) {
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
            for (Channel* channel: secondaryPlaybackChannels()) {
                  if (value)
                        seq->stopNotes(channel->channel());
                  channel->setSolo(value);
                  }
            }

      // TODO: duplicated code - look at how to eliminate
      // NOTE: We have here the same iteration that is used for
      // updateTracks in the mixer. SO definitely still scope for code
      // reduction. And this method needs to either live somewhere
      // else, Mixer maybe, or (maybe AND) be a static/class method.
      // 

      //Go through all channels so that all not being soloed are mute

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

//MARK:- helper methods

template <class ChannelWriter, class ChannelReader>
void MixerTrackItem:: adjustValue(int newValue, ChannelReader reader, ChannelWriter writer)
{

      if (!isPart()) {
            // only one channel, the easy case - just make a direct adjustment
            writer(newValue, _channel);
            return;
      }

      // multiple channels and the OVERALL value has been changed
      // make adjustments depending on the OVERALL mode

      MixerVolumeMode mode = Mixer::getOptions()->mode();
      int primaryDiff = 0;
      bool upperClip = false;
      bool lowerClip = false;

      primaryDiff = newValue - reader(channel());

      // update the secondary channels
      for (Channel* channel: secondaryPlaybackChannels()) {

            switch (mode) {
                  case MixerVolumeMode::Override:
                        // all secondary channels just get newValue
                        writer(newValue, channel);
                        break;

                  case MixerVolumeMode::Ratio: {
                        // secondary channels get same increase / decrease as primary value
                        int oldValue = reader(channel);
                        int relativeValue = oldValue + primaryDiff;
                        upperClip = relativeValue > 127;
                        lowerClip = relativeValue < 0;
                        writer(max(0, min(127, relativeValue)), channel);
                        break;
                  }

                  case MixerVolumeMode::PrimaryInstrument:
                        // secondary channels are not touched
                        break;
            }
      }

      if (upperClip || lowerClip) {
            //TODO: clipping - scope to explore different behaviors here - but for now NO OP
      }

      writer(newValue, channel());
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

//MARK:- MixerTreeWidgetItem class

// Don't like the way in which I'm passing so many different vars through here. Doesn't
// feel like good design. At the moment just mirroring the original code though.
// Don't yet fully undertand the parts / instuments data structure, so just replicating
// what the old mixer code did. It does feel as if there should be a class that is
// a comprehensive descriptor and that captures all the gubbins being passed back and
// forth.

MixerTreeWidgetItem::MixerTreeWidgetItem(Part* part, Score* score, QTreeWidget* treeWidget)
      {
      mixerTrackItem = new MixerTrackItem(part, score);
      setText(0, part->partName());
      setToolTip(0, part->partName());

      // check for secondary channels and add MixerTreeWidgetItem children if required
      const InstrumentList* partInstrumentList = part->instruments(); //Add per channel tracks
      
      // partInstrumentList is of type: map<const int, Instrument*>
      for (auto partInstrumentListItem = partInstrumentList->begin(); partInstrumentListItem != partInstrumentList->end(); ++partInstrumentListItem) {
            
            Instrument* instrument = partInstrumentListItem->second;
            if (instrument->channel().size() <= 1)
                  continue;
            
            for (int i = 0; i < instrument->channel().size(); ++i) {
                  Channel* channel = instrument->playbackChannel(i, score->masterScore());
                  MixerTreeWidgetItem* child = new MixerTreeWidgetItem(channel, instrument, part);
                  addChild(child);
                  treeWidget->setItemWidget(child, 1, new MixerTrackChannel(child));
                  }
            }
      }

MixerTreeWidgetItem::MixerTreeWidgetItem(Channel* channel, Instrument* instrument, Part* part)
      {
      setText(0, channel->name());
      setToolTip(0, QString("%1 - %2").arg(part->partName()).arg(channel->name()));
      mixerTrackItem = new MixerTrackItem(MixerTrackItem::TrackType::CHANNEL, part, instrument, channel);
      }
      
}

