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

MixerTrackItem::MixerTrackItem(TrackType trackType, Part* part, Instrument* instr, Channel *chan)
      :_trackType(trackType), _part(part), _instrument(instr), _channel(chan)
      {
      }

//---------------------------------------------------------
//   midiMap
//---------------------------------------------------------

MidiMapping *MixerTrackItem::midiMap()
      {
      return _part->masterScore()->midiMapping(chan()->channel());
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

int MixerTrackItem::color()
      {
      return _trackType ==TrackType::PART ? _part->color() : _channel->color();
      }


char MixerTrackItem::getVolume()
      {
      return chan()->volume();
      }

char MixerTrackItem::getPan()
      {
      return chan()->pan();
      }

bool MixerTrackItem::getMute()
      {
      return chan()->mute();
      }

bool MixerTrackItem::getSolo()
      {
      return chan()->solo();
      }

// MixerTrackItem settters - when a change is made to underlying channel a propertyChange()
// will be sent to any registered listeners

void MixerTrackItem::finalizeVolume(Channel* channel, int value)
      {
      if (channel->volume() == value)
            return;

      value = (value > 127) ? 127 : value;
      value = (value < 0) ? 0 : value;

      channel->setVolume(value);
      seq->setController(channel->channel(), CTRL_VOLUME, channel->volume());
      }

void MixerTrackItem::setVolume(char value)
      {

      if (!isPart()) {
            if (_channel->volume() == value)
                  return;
            finalizeVolume(_channel, value);
            return;
      }

      // case where we're operating a slider "manages" some sub-channels
      // test for different policies as to how the secondary channels
      // respond to changes made by the slider for the primary channel

      //TODO: can't put a brake on the slider, but there could be a
      // spring back if we hit a ceiling - so after we let go of the slider
      // it could return to the point where the limit was hit

      MixerVolumeMode mode = Mixer::getOptions()->mode();
      bool primaryChannel = true;
      int primaryDiff = 0;
      for (Channel* channel: subChannels()) {
            if (primaryChannel)
                  primaryDiff = value - double(channel->volume());

            switch (mode) {
                  case MixerVolumeMode::Override:
                        // set primary channel and all secondary channels to the same value
                        finalizeVolume(channel, value);
                        break;

                  case MixerVolumeMode::Ratio:
                        // adjust the secondary channels by the same amount, but starting where
                        // they are already at - not a RATIO at all!!
                        if (primaryChannel)
                              finalizeVolume(channel, value);
                        else {
                              int oldValue = channel->volume();
                              finalizeVolume(channel, oldValue + primaryDiff);
                              }
                        break;

                  case MixerVolumeMode::PrimaryInstrument:
                        // adjust ONLY the primary channel
                        if (primaryChannel)
                              finalizeVolume(channel, value);
                        break;
                  }
            primaryChannel = false;
            }
      }

//---------------------------------------------------------
//   setPan
//---------------------------------------------------------

void MixerTrackItem::setPan(char value)
      {
      //      char v = (char)qBound(0, (int)((value + 180.0) / 360.0 * 128.0), 127);

      if (_trackType == TrackType::PART) {
            const InstrumentList* il = _part->instruments();
            for (auto it = il->begin(); it != il->end(); ++it) {
                  Instrument* instr = it->second;
                  for (const Channel* instrChan: instr->channel()) {
                        Channel* chan = playbackChannel(instrChan);
                        if (chan->pan() != value) {
                              chan->setPan(value);
                              seq->setController(chan->channel(), CTRL_PANPOT, chan->pan());
                              }
                        }
                  }
            }
      else {
            if (_channel->pan() != value) {
                  _channel->setPan(value);
                  seq->setController(_channel->channel(), CTRL_PANPOT, _channel->pan());
                  }
            }
      }

//---------------------------------------------------------
//   setChorus
//---------------------------------------------------------

void MixerTrackItem::setChorus(char value)
      {
      //      char v = (char)qBound(0, (int)(value * 128.0 / 100.0), 127);

      if (_trackType == TrackType::PART) {
            const InstrumentList* il = _part->instruments();
            for (auto it = il->begin(); it != il->end(); ++it) {
                  Instrument* instr = it->second;
                  for (const Channel* instrChan: instr->channel()) {
                        Channel* chan = playbackChannel(instrChan);
                        if (chan->chorus() != value) {
                              chan->setChorus(value);
                              seq->setController(chan->channel(), CTRL_CHORUS_SEND, chan->chorus());
                              }
                        }
                  }
            }
      else {
            if (_channel->chorus() != value) {
                  _channel->setChorus(value);
                  seq->setController(_channel->channel(), CTRL_CHORUS_SEND, _channel->chorus());
                  }
            }
      }

//---------------------------------------------------------
//   setReverb
//---------------------------------------------------------

void MixerTrackItem::setReverb(char value)
      {
      //      char v = (char)qBound(0, (int)(value * 128.0 / 100.0), 127);

      if (_trackType == TrackType::PART) {
            const InstrumentList* il = _part->instruments();
            for (auto it = il->begin(); it != il->end(); ++it) {
                  Instrument* instr = it->second;
                  for (const Channel* instrChan: instr->channel()) {
                        Channel* chan = playbackChannel(instrChan);
                        if (chan->reverb() != value) {
                              chan->setReverb(value);
                              seq->setController(chan->channel(), CTRL_REVERB_SEND, chan->reverb());
                              }
                        }
                  }
            }
      else {
            if (_channel->reverb() != value) {
                  _channel->setReverb(value);
                  seq->setController(_channel->channel(), CTRL_REVERB_SEND, _channel->reverb());
                  }
            }
      }

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void MixerTrackItem::setColor(int valueRgb)
      {
      if (_trackType == TrackType::PART) {
            _part->setColor(valueRgb);

            const InstrumentList* il = _part->instruments();
            for (auto it = il->begin(); it != il->end(); ++it) {
                  Instrument* instr = it->second;
                  for (const Channel* instrChan: instr->channel()) {
                        Channel* chan = playbackChannel(instrChan);
                        chan->setColor(valueRgb);
                        }
                  }
            }
      else {
            _channel->setColor(valueRgb);
            }
      }

//---------------------------------------------------------
//   setMute
//---------------------------------------------------------

void MixerTrackItem::setMute(bool value)
      {
      if (_trackType == TrackType::PART) {
            const InstrumentList* il = _part->instruments();
            for (auto it = il->begin(); it != il->end(); ++it) {
                  Instrument* instr = it->second;
                  for (const Channel* instrChan: instr->channel()) {
                        Channel* chan = playbackChannel(instrChan);
                        if (value)
                              seq->stopNotes(chan->channel());
                        chan->setMute(value);
                        }
                  }
            }
      else {
            if (value)
                  seq->stopNotes(_channel->channel());
            _channel->setMute(value);
            }
      }

//---------------------------------------------------------
//   setSolo
//---------------------------------------------------------

void MixerTrackItem::setSolo(bool value)
      {
      if (_trackType == TrackType::PART) {
            const InstrumentList* il = _part->instruments();
            for (auto it = il->begin(); it != il->end(); ++it) {
                  Instrument* instr = it->second;
                  for (const Channel* instrChan: instr->channel()) {
                        Channel* chan = playbackChannel(instrChan);
                        chan->setSolo(value);
                        }
                  }
            }
      else {
            _channel->setSolo(value);
            }

      //Go through all channels so that all not being soloed are mute
      int numSolo = 0;
      for (Part* p : _part->score()->parts()) {
            const InstrumentList* il = p->instruments();
            for (auto i = il->begin(); i != il->end(); ++i) {
                  const Instrument* instr = i->second;
                  for (const Channel* instrChan: instr->channel()) {
                        Channel* a = playbackChannel(instrChan);
                        if (a->solo()) {
                              numSolo++;
                              }
                        }
                  }
            }

      for (Part* p : _part->score()->parts()) {
            const InstrumentList* il = p->instruments();
            for (auto i = il->begin(); i != il->end(); ++i) {
                  const Instrument* instr = i->second;
                  for (const Channel* instrChan: instr->channel()) {
                        Channel* a = playbackChannel(instrChan);
                        if (numSolo == 0) {
                              a->setSoloMute(false);
                              }
                        else {
                              a->setSoloMute(!a->solo());
                              if (a->soloMute()) {
                                    seq->stopNotes(a->channel());
                                    }
                              }
                        }
                  }
            }
      }

//MARK:- helper methods

bool MixerTrackItem::isPart()
      {
      return _trackType == TrackType::PART;
      }


QList<Channel*> MixerTrackItem::subChannels()
      {
      if (!isPart()) {
            return QList<Channel*> { _channel };
            }

      QList<Channel*> channels;

      //TODO: duplication between here and in Mixer::updateTracks
      const InstrumentList* instrumentList = _part->instruments();
      for (auto item = instrumentList->begin(); item != instrumentList->end(); ++item) {
            Instrument* instrument = item->second;
            for (const Channel* instrumentChannel: instrument->channel()) {
                  Channel* channel = playbackChannel(instrumentChannel);
                  channels.append(channel);
                  }
            }
      return channels;
      }

}

