/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "excerpt.h"
#include "instrument.h"
#include "masterscore.h"
#include "part.h"
#include "score.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
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
    isSimpleMidiMapping = true;
    rebuildMidiMapping();

    std::vector<bool> drum;
    drum.reserve(_midiMapping.size());
    for (Part* part : parts()) {
        for (const auto& pair : part->instruments()) {
            const Instrument* instr = pair.second;
            for (size_t j = 0; j < instr->channel().size(); ++j) {
                drum.push_back(instr->useDrumset());
            }
        }
    }
    int lastChannel  = -1;   // port*16+channel
    int lastDrumPort = -1;
    size_t index = 0;
    for (const MidiMapping& m : _midiMapping) {
        if (index >= drum.size()) {
            break;
        }
        if (drum[index]) {
            lastDrumPort++;
            if (m.port() != lastDrumPort) {
                isSimpleMidiMapping = false;
                return;
            }
        } else {
            lastChannel++;
            if (lastChannel % 16 == 9) {
                lastChannel++;
            }
            int p = lastChannel / 16;
            int c = lastChannel % 16;
            if (m.port() != p || m.channel() != c) {
                isSimpleMidiMapping = false;
                return;
            }
        }
        index++;
    }
}

//---------------------------------------------------------
//   getNextFreeMidiMapping
//---------------------------------------------------------

int MasterScore::getNextFreeMidiMapping(std::set<int>& occupiedMidiChannels, unsigned int& searchMidiMappingFrom, int p, int ch)
{
    if (ch != -1 && p != -1) {
        return p * 16 + ch;
    } else if (ch != -1 && p == -1) {
        for (int port = 0;; port++) {
            if (!mu::contains(occupiedMidiChannels, port * 16 + ch)) {
                occupiedMidiChannels.insert(port * 16 + ch);
                return port * 16 + ch;
            }
        }
    } else if (ch == -1 && p != -1) {
        for (int channel = 0; channel < 16; channel++) {
            if (channel != 9 && !mu::contains(occupiedMidiChannels, p * 16 + channel)) {
                occupiedMidiChannels.insert(p * 16 + channel);
                return p * 16 + channel;
            }
        }
    }

    for (;; searchMidiMappingFrom++) {
        if (searchMidiMappingFrom % 16 != 9 && !mu::contains(occupiedMidiChannels, int(searchMidiMappingFrom))) {
            occupiedMidiChannels.insert(searchMidiMappingFrom);
            return searchMidiMappingFrom;
        }
    }
}

//---------------------------------------------------------
//   getNextFreeDrumMidiMapping
//---------------------------------------------------------

int MasterScore::getNextFreeDrumMidiMapping(std::set<int>& occupiedMidiChannels)
{
    for (int i = 0;; i++) {
        if (!mu::contains(occupiedMidiChannels, i * 16 + 9)) {
            occupiedMidiChannels.insert(i * 16 + 9);
            return i * 16 + 9;
        }
    }
}

//---------------------------------------------------------
//   rebuildExcerptsMidiMapping
//---------------------------------------------------------

void MasterScore::rebuildExcerptsMidiMapping()
{
    for (Excerpt* ex : excerpts()) {
        for (Part* p : ex->excerptScore()->parts()) {
            const Part* masterPart = p->masterPart();
            if (!masterPart->score()->isMaster()) {
                LOGW() << "rebuildExcerptsMidiMapping: no part in master score is linked with " << p->partName();
                continue;
            }
            assert(p->instruments().size() == masterPart->instruments().size());
            for (const auto&[tick, iMaster] : masterPart->instruments()) {
                Instrument* iLocal = p->instrument(Fraction::fromTicks(tick));
                const size_t nchannels = iMaster->channel().size();
                if (iLocal->channel().size() != nchannels) {
                    // may happen, e.g., if user changes an instrument
                    (*iLocal) = (*iMaster);
                    continue;
                }
                for (size_t c = 0; c < nchannels; ++c) {
                    InstrChannel* cLocal = iLocal->channel(static_cast<int>(c));
                    const InstrChannel* cMaster = iMaster->channel(static_cast<int>(c));
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
        for (const auto& pair : part->instruments()) {
            const Instrument* instr = pair.second;
            for (InstrChannel* channel : instr->channel()) {
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
    int mappingSize = int(_midiMapping.size());
    for (int index = 0; index < mappingSize; index++) {
        Part* part = midiMapping(index)->part();
        if (!mu::contains(parts(), part)) {
            removeOffset++;
            continue;
        }
        // Not all channels could exist
        bool channelExists = false;
        for (const auto& pair : part->instruments()) {
            const Instrument* instr = pair.second;
            channelExists = (_midiMapping[index].articulation()->channel() != -1
                             && mu::contains(instr->channel(), _midiMapping[index].masterChannel)
                             && !(_midiMapping[index].port() == -1 && _midiMapping[index].channel() == -1));
            if (channelExists) {
                break;
            }
        }
        if (!channelExists) {
            removeOffset++;
            continue;
        }
        // Let's do a left shift by 'removeOffset' items if necessary
        if (index != 0 && removeOffset != 0) {
            _midiMapping[index - removeOffset] = std::move(_midiMapping[index]);

            const int chanVal = _midiMapping[index - removeOffset].articulation()->channel();
            _midiMapping[index - removeOffset].articulation()->setChannel(chanVal - removeOffset);
        }
    }
    // We have 'removeOffset' deleted instruments, let's remove their mappings
    for (int index = 0; index < removeOffset; index++) {
        _midiMapping.pop_back();
    }
}

//---------------------------------------------------------
//   updateMidiMapping
//   Add mappings to new instruments and repair existing ones
//---------------------------------------------------------

int MasterScore::updateMidiMapping()
{
    int maxport = 0;
    std::set<int> occupiedMidiChannels;// each entry is port*16+channel, port range: 0-inf, channel: 0-15
    unsigned int searchMidiMappingFrom = 0;           // makes getting next free MIDI mapping faster

    for (const MidiMapping& mm :_midiMapping) {
        if (mm.port() == -1 || mm.channel() == -1) {
            continue;
        }
        occupiedMidiChannels.insert(static_cast<int>(mm.port()) * 16 + (int)mm.channel());
        if (maxport < mm.port()) {
            maxport = mm.port();
        }
    }

    for (Part* part : parts()) {
        for (const auto& pair : part->instruments()) {
            const Instrument* instr = pair.second;
            bool drum = instr->useDrumset();
            for (InstrChannel* channel : instr->channel()) {
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
                        const int nm
                            = getNextFreeMidiMapping(occupiedMidiChannels, searchMidiMappingFrom, -1,
                                                     _midiMapping[channel->channel()].channel());
                        _midiMapping[channel->channel()]._port = nm / 16;
                    } else if (_midiMapping[channel->channel()].channel() == -1) {
                        if (drum) {
                            _midiMapping[channel->channel()]._port = getNextFreeDrumMidiMapping(occupiedMidiChannels) / 16;
                            _midiMapping[channel->channel()]._channel = 9;
                            continue;
                        }
                        int nm = getNextFreeMidiMapping(occupiedMidiChannels, searchMidiMappingFrom,
                                                        _midiMapping[channel->channel()].port());
                        _midiMapping[channel->channel()]._port    = nm / 16;
                        _midiMapping[channel->channel()]._channel = nm % 16;
                    }
                    continue;
                }

                int midiPort;
                int midiChannel;
                if (drum) {
                    midiPort = getNextFreeDrumMidiMapping(occupiedMidiChannels) / 16;
                    midiChannel = 9;
                } else {
                    int nm = getNextFreeMidiMapping(occupiedMidiChannels, searchMidiMappingFrom);
                    midiPort    = nm / 16;
                    midiChannel = nm % 16;
                }

                if (midiPort > maxport) {
                    maxport = midiPort;
                }

                addMidiMapping(channel, part, midiPort, midiChannel);
            }
        }
    }
    return maxport;
}

//---------------------------------------------------------
//   addMidiMapping
//---------------------------------------------------------

void MasterScore::addMidiMapping(InstrChannel* channel, Part* part, int midiPort, int midiChannel)
{
    if (!part->score()->isMaster()) {
        return;
    }

    MidiMapping mm;
    mm._part = part;
    mm.masterChannel = channel;
    mm._articulation.reset(new InstrChannel(*channel));
    mm.link = PartChannelSettingsLink(mm.articulation(), mm.masterChannel, /* excerpt */ false);

    mm._port = midiPort;
    mm._channel = midiChannel;

    const int mscoreChannel = int(_midiMapping.size());
    mm._articulation->setChannel(mscoreChannel);
    mm.masterChannel->setChannel(mscoreChannel);

    _midiMapping.push_back(std::move(mm));
}

//---------------------------------------------------------
//   updateMidiMapping
//---------------------------------------------------------

void MasterScore::updateMidiMapping(InstrChannel* channel, Part* part, int midiPort, int midiChannel)
{
    const int c = channel->channel();
    if (c < 0) {
        return;
    }
    if (c >= int(masterScore()->midiMapping().size())) {
        LOGD("Can't set midi channel: midiMapping is empty!");
        return;
    }
    MidiMapping& mm = _midiMapping[c];

    if (midiChannel != -1) {
        mm._channel = midiChannel;
    }
    if (midiPort != -1) {
        mm._port = midiPort;
    }
    if (part) {
        mm._part = part->masterPart();
    }
}
} // namespace mu::engraving
