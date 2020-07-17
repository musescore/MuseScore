//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#include "notationmididata.h"

#include <cmath>

#include "log.h"

#include "libmscore/score.h"
#include "libmscore/tempo.h"
#include "libmscore/part.h"
#include "libmscore/instrument.h"
#include "libmscore/repeatlist.h"

#include "audio/midi/event.h" //! TODO Remove me

using namespace mu::domain::notation;
using namespace mu::audio::midi;

static EventType convertType(int type)
{
    switch (type) {
    case Ms::ME_NOTEON:       return ME_NOTEON;
    case Ms::ME_NOTEOFF:      return ME_NOTEOFF;
    case Ms::ME_CONTROLLER:   return ME_CONTROLLER;
    case Ms::ME_PITCHBEND:    return ME_PITCHBEND;
    case Ms::ME_META:         return ME_META;

    case Ms::ME_TICK1:        return ME_INVALID;
    case Ms::ME_TICK2:        return ME_INVALID;
    default: {
        LOGE() << "unknown midi type: " << type;
    }
    }
    return ME_INVALID;
}

NotationMidiData::NotationMidiData(IGetScore* getScore)
    : m_getScore(getScore)
{
}

MidiStream NotationMidiData::midiStream() const
{
    Ms::Score* score = m_getScore->score();
    if (!score) {
        return MidiStream();
    }

    makeInitData(m_stream.initData, score);

    m_stream.request.onReceive(this, [this](uint32_t tick) {
        UNUSED(tick);
        m_stream.stream.close();
    });

    return m_stream;
}

void NotationMidiData::makeInitData(MidiData& data, Ms::Score* score) const
{
    MetaInfo meta;
    makeMetaInfo(meta, score);

    data.division = Ms::MScore::division;
    data.tracks.resize(meta.tracksCount);

    Ms::EventMap eventMap;
    makeEventMap(eventMap, score);
    fillTracks(data.tracks, eventMap, meta);

    fillTempoMap(data.tempomap, score);

    //fillMetronome(stream->initData.metronome, score, midiSpec);
}

void NotationMidiData::makeEventMap(Ms::EventMap& eventMap, Ms::Score* score) const
{
//    int unrenderedUtick = renderEventsStatus.occupiedRangeEnd(utick);
//    while (unrenderedUtick - utick < minUtickBufferSize) {
//        const MidiRenderer::Chunk chunk = midi.getChunkAt(unrenderedUtick);
//        if (!chunk) {
//            break;
//        }
//        renderChunk(chunk, &events);
//        unrenderedUtick = renderEventsStatus.occupiedRangeEnd(utick);
//    }

    score->masterScore()->setExpandRepeats(true);
    score->renderMidi(&eventMap, Ms::SynthesizerState());
}

void NotationMidiData::makeMetaInfo(MetaInfo& meta, const Ms::Score* score) const
{
    auto parts = score->parts();

    meta.tracksCount = parts.size();

    auto bankForInstrument = [](const Ms::Instrument* instr) {
                                 //! NOTE Temporary solution
                                 if (instr->useDrumset()) {
                                     return 128;
                                 }
                                 return 0;
                             };

    for (int pi = 0; pi < parts.size(); ++pi) {
        const Ms::Part* part = parts.at(pi);

        const Ms::InstrumentList* instList = part->instruments();
        for (auto it = instList->cbegin(); it != instList->cend(); ++it) {
            const Ms::Instrument* instrument = it->second;

            uint16_t bank = bankForInstrument(instrument);

            for (const Ms::Channel* ch : instrument->channel()) {
                ChanInfo chi;
                chi.trackIdx = pi;
                chi.bank = bank;
                chi.program = ch->program();

                meta.channels.insert({ ch->channel(), chi });
            }
        }
    }
}

void NotationMidiData::fillTracks(std::vector<audio::midi::Track>& tracks, const Ms::EventMap& eventMap,
                                  const MetaInfo& meta) const
{
    uint16_t ch_num = 1; //! NOTE channel 0 reserved for metronome
    auto findOrAddChannel = [&ch_num](audio::midi::Track& t, const ChanInfo& chi) -> audio::midi::Channel& {
                                for (auto& ch : t.channels) {
                                    if (ch.program == chi.program && ch.bank == chi.bank) {
                                        return ch;
                                    }
                                }

                                audio::midi::Channel ch;
                                ch.num = ch_num;
                                ch.program = chi.program;
                                ch.bank = chi.bank;

                                ++ch_num;

                                t.channels.push_back(std::move(ch));
                                return t.channels.back();
                            };

    for (const auto& evp : eventMap) {
        int tick = evp.first;
        const Ms::NPlayEvent ev = evp.second;

        if (ev.type() == Ms::ME_CONTROLLER && ev.controller() == 2) {
            //! TODO Understand why these events
            continue;
        }

        auto foundIt = meta.channels.find(ev.channel());
        if (foundIt == meta.channels.end()) {
            Q_ASSERT(foundIt != meta.channels.end());
            continue;
        }

        const ChanInfo& chi = foundIt->second;

        audio::midi::Track& track = tracks.at(chi.trackIdx);
        audio::midi::Channel& ch = findOrAddChannel(track, chi);

        audio::midi::EventType etype = convertType(ev.type());
        if (audio::midi::ME_INVALID == etype) {
            continue;
        } else if (audio::midi::ME_META == etype) {
            continue;
        } else {
            audio::midi::Event e
            { static_cast<uint32_t>(tick),
              etype,
              ev.dataA(), ev.dataB()
            };

            ch.events.push_back(std::move(e));
        }
    }
}

void NotationMidiData::fillTempoMap(std::map<uint32_t, uint32_t>& tempos, const Ms::Score* score) const
{
    Ms::TempoMap* tempomap = score->tempomap();
    qreal relTempo = tempomap->relTempo();
    for (const Ms::RepeatSegment* rs : score->repeatList()) {
        int startTick = rs->tick, endTick = startTick + rs->len();
        int tickOffset = rs->utick - rs->tick;

        auto se = tempomap->lower_bound(startTick);
        auto ee = tempomap->lower_bound(endTick);
        for (auto it = se; it != ee; ++it) {
            //
            // compute midi tempo: microseconds / quarter note
            //
            uint32_t tempo = (uint32_t)lrint((1.0 / (it->second.tempo * relTempo)) * 1000000.0);

            tempos.insert({ it->first + tickOffset, tempo });
        }
    }
}
