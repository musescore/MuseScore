//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================


#include "score.h"
#include "excerpt.h"
#include "instrument.h"
#include "part.h"

namespace Ms {

//---------------------------------------------------------
//   rebuildMidiMapping
//---------------------------------------------------------

void MasterScore::rebuildMidiMapping()
      {
      Score* playbackScore = _playbackScore ? _playbackScore : this;
      setPlaybackScore(nullptr);

      removeDeletedMidiMapping();
      int maxport = updateMidiMapping();
      reorderMidiMapping();
      rebuildExcerptsMidiMapping();
      masterScore()->setMidiPortCount(maxport);

      setPlaybackScore(playbackScore);
      }

//---------------------------------------------------------
//   checkMidiMapping
//   midi mapping is simple if all ports and channels
//   don't decrease and don't have 'holes' except drum tracks
//---------------------------------------------------------

void MasterScore::checkMidiMapping()
      {
      isSimpleMidiMaping = true;
      rebuildMidiMapping();

      QList<bool> drum;
      drum.reserve(_midiMapping.size());
      for (Part* part : parts()) {
            const InstrumentList* il = part->instruments();
            for (auto i = il->begin(); i != il->end(); ++i) {
                  const Instrument* instr = i->second;
                  for (int j = 0; j < instr->channel().size(); ++j)
                        drum.append(instr->useDrumset());
                  }
            }
      int lastChannel  = -1; // port*16+channel
      int lastDrumPort = -1;
      int index = 0;
      for (const MidiMapping& m : _midiMapping) {
            if (index >= drum.size())
                  break;
            if (drum[index]) {
                  lastDrumPort++;
                  if (m.port() != lastDrumPort) {
                        isSimpleMidiMaping = false;
                        return;
                        }
                  }
            else {
                  lastChannel++;
                  if (lastChannel % 16 == 9)
                        lastChannel++;
                  int p = lastChannel / 16;
                  int c = lastChannel % 16;
                  if (m.port() != p || m.channel() != c) {
                        isSimpleMidiMaping = false;
                        return;
                        }
                  }
            index++;
            }
      }

//---------------------------------------------------------
//   getNextFreeMidiMapping
//---------------------------------------------------------

int MasterScore::getNextFreeMidiMapping(int p, int ch)
      {
      if (ch != -1 && p != -1)
            return p*16+ch;

      else if (ch != -1 && p == -1) {
            for (int port = 0;; port++) {
                  if (!occupiedMidiChannels.contains(port*16+ch)) {
                        occupiedMidiChannels.insert(port*16+ch);
                        return port*16+ch;
                        }
                  }
            }
      else if (ch == -1 && p != -1) {
            for (int channel = 0; channel < 16; channel++) {
                  if (channel != 9 && !occupiedMidiChannels.contains(p*16+channel)) {
                        occupiedMidiChannels.insert(p*16+channel);
                        return p*16+channel;
                        }
                  }
            }

      for (;;searchMidiMappingFrom++) {
            if (searchMidiMappingFrom % 16 != 9 && !occupiedMidiChannels.contains(searchMidiMappingFrom)) {
                  occupiedMidiChannels.insert(searchMidiMappingFrom);
                  return searchMidiMappingFrom;
                  }
            }
      }

//---------------------------------------------------------
//   getNextFreeDrumMidiMapping
//---------------------------------------------------------

int MasterScore::getNextFreeDrumMidiMapping()
      {
      for (int i = 0;; i++) {
            if (!occupiedMidiChannels.contains(i*16+9)) {
                  occupiedMidiChannels.insert(i*16+9);
                  return i*16+9;
                  }
            }
      }

//---------------------------------------------------------
//   rebuildExcerptsMidiMapping
//---------------------------------------------------------

void MasterScore::rebuildExcerptsMidiMapping()
      {
      for (Excerpt* ex : excerpts()) {
            for (Part* p : ex->partScore()->parts()) {
                  const Part* masterPart = p->masterPart();
                  if (!masterPart->score()->isMaster()) {
                        qWarning("reorderMidiMapping: no part in master score is linked");
                        continue;
                        }
                  Q_ASSERT(p->instruments()->size() == masterPart->instruments()->size());
                  for (const auto& item : *masterPart->instruments()) {
                        const Instrument* iMaster = item.second;
                        const int tick = item.first;
                        Instrument* iLocal = p->instrument(Fraction::fromTicks(tick));
                        Q_ASSERT(iLocal->channel().size() == iMaster->channel().size());
                        const int nchannels = iLocal->channel().size();
                        for (int c = 0; c < nchannels; ++c) {
                              Channel* cLocal = iLocal->channel(c);
                              const Channel* cMaster = iMaster->channel(c);
                              cLocal->setChannel(cMaster->channel());
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   reorderMidiMapping
//   Set mappings in order you see in Add->Instruments
//---------------------------------------------------------

void MasterScore::reorderMidiMapping()
      {
      using std::swap;
      int sequenceNumber = 0;
      for (Part* part : parts()) {
            const InstrumentList* il = part->instruments();
            for (auto i = il->begin(); i != il->end(); ++i) {
                  const Instrument* instr = i->second;
                  for (Channel* channel : instr->channel()) {
                        if (!(_midiMapping[sequenceNumber].part() == part
                              && _midiMapping[sequenceNumber].masterChannel == channel)) {
                              int shouldBe = channel->channel();
                              swap(_midiMapping[sequenceNumber], _midiMapping[shouldBe]);
                              _midiMapping[sequenceNumber].articulation()->setChannel(sequenceNumber);
                              channel->setChannel(sequenceNumber);
                              _midiMapping[shouldBe].articulation()->setChannel(shouldBe);
                              }
                        sequenceNumber++;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   removeDeletedMidiMapping
//   Remove mappings to deleted instruments
//---------------------------------------------------------

void MasterScore::removeDeletedMidiMapping()
      {
      int removeOffset = 0;
      int mappingSize = _midiMapping.size();
      for (int index = 0; index < mappingSize; index++) {
            Part* p = midiMapping(index)->part();
            if (!parts().contains(p)) {
                  removeOffset++;
                  continue;
                  }
            // Not all channels could exist
            bool channelExists = false;
            const InstrumentList* il = p->instruments();
            for (auto i = il->begin(); i != il->end() && !channelExists; ++i) {
                  const Instrument* instr = i->second;
                  channelExists = (_midiMapping[index].articulation()->channel() != -1
                      && instr->channel().contains(_midiMapping[index].masterChannel)
                      && !(_midiMapping[index].port() == -1 && _midiMapping[index].channel() == -1));
                  if (channelExists)
                        break;
                  }
            if (!channelExists) {
                  removeOffset++;
                  continue;
                  }
            // Let's do a left shift by 'removeOffset' items if necessary
            if (index != 0 && removeOffset != 0) {
                  _midiMapping[index-removeOffset] = std::move(_midiMapping[index]);

                  const int chanVal = _midiMapping[index-removeOffset].articulation()->channel();
                  _midiMapping[index-removeOffset].articulation()->setChannel(chanVal - removeOffset);
                  }
            }
      // We have 'removeOffset' deleted instruments, let's remove their mappings
      for (int index = 0; index < removeOffset; index++)
            _midiMapping.pop_back();
      }

//---------------------------------------------------------
//   updateMidiMapping
//   Add mappings to new instruments and repair existing ones
//---------------------------------------------------------

int MasterScore::updateMidiMapping()
      {
      int maxport = 0;
      occupiedMidiChannels.clear();
      searchMidiMappingFrom = 0;
      occupiedMidiChannels.reserve(_midiMapping.size()); // Bringing down the complexity of insertion to amortized O(1)

      for (const MidiMapping& mm :_midiMapping) {
            if (mm.port() == -1 || mm.channel() == -1)
                  continue;
            occupiedMidiChannels.insert((int)(mm.port())*16+(int)mm.channel());
            if (maxport < mm.port())
                  maxport = mm.port();
            }

      for (Part* part : parts()) {
            const InstrumentList* il = part->instruments();
            for (auto i = il->begin(); i != il->end(); ++i) {
                  const Instrument* instr = i->second;
                  bool drum = instr->useDrumset();
                  for (Channel* channel : instr->channel()) {
                        bool channelExists = false;
                        for (const MidiMapping& mapping: _midiMapping) {
                              if (channel == mapping.masterChannel && channel->channel() != -1) {
                                    channelExists = true;
                                    break;
                                    }
                              }
                        // Channel could already exist, but have unassigned port or channel. Repair and continue
                        if (channelExists) {
                              if (_midiMapping[channel->channel()].port() == -1) {
                                    const int nm = getNextFreeMidiMapping(-1, _midiMapping[channel->channel()].channel());
                                    _midiMapping[channel->channel()]._port = nm / 16;
                                    }
                              else if (_midiMapping[channel->channel()].channel() == -1) {
                                    if (drum) {
                                          _midiMapping[channel->channel()]._port = getNextFreeDrumMidiMapping() / 16;
                                          _midiMapping[channel->channel()]._channel = 9;
                                          continue;
                                          }
                                    int nm = getNextFreeMidiMapping(_midiMapping[channel->channel()].port());
                                    _midiMapping[channel->channel()]._port    = nm / 16;
                                    _midiMapping[channel->channel()]._channel = nm % 16;
                                    }
                              continue;
                              }

                        int midiPort;
                        int midiChannel;
                        if (drum) {
                              midiPort = getNextFreeDrumMidiMapping() / 16;
                              midiChannel = 9;
                              }
                        else {
                              int nm = getNextFreeMidiMapping();
                              midiPort    = nm / 16;
                              midiChannel = nm % 16;
                              }

                        if (midiPort > maxport)
                              maxport = midiPort;

                        addMidiMapping(channel, part, midiPort, midiChannel);
                        }
                  }
            }
      return maxport;
      }

//---------------------------------------------------------
//   addMidiMapping
//---------------------------------------------------------

void MasterScore::addMidiMapping(Channel* channel, Part* part, int midiPort, int midiChannel)
      {
      if (!part->score()->isMaster())
            return;

      MidiMapping mm;
      mm._part = part;
      mm.masterChannel = channel;
      mm._articulation.reset(new Channel(*channel));
      mm.link = PartChannelSettingsLink(mm.articulation(), mm.masterChannel, /* excerpt */ false);

      mm._port = midiPort;
      mm._channel = midiChannel;

      const int mscoreChannel = _midiMapping.size();
      mm._articulation->setChannel(mscoreChannel);
      mm.masterChannel->setChannel(mscoreChannel);

      _midiMapping.push_back(std::move(mm));
      }

//---------------------------------------------------------
//   updateMidiMapping
//---------------------------------------------------------

void MasterScore::updateMidiMapping(Channel* channel, Part* part, int midiPort, int midiChannel)
      {
      const int c = channel->channel();
      if (c < 0)
            return;
      if (c >= int(masterScore()->midiMapping().size())) {
            qDebug("Can't set midi channel: midiMapping is empty!");
            return;
            }
      MidiMapping& mm = _midiMapping[c];

      if (midiChannel != -1)
            mm._channel = midiChannel;
      if (midiPort != -1)
            mm._port = midiPort;
      if (part)
            mm._part = part->masterPart();
      }

} // namespace Ms

