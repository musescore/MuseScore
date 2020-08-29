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

#ifndef MU_MIDI_SEQUENCER_H
#define MU_MIDI_SEQUENCER_H

#include <memory>
#include <vector>
#include <map>
#include <cstdint>
#include <functional>
#include <chrono>
#include <mutex>

#include "../isequencer.h"
#include "../miditypes.h"
#include "modularity/ioc.h"
#include "../isynthesizersregister.h"
#include "async/asyncable.h"

namespace mu {
namespace midi {
class Sequencer : public ISequencer, public async::Asyncable
{
    INJECT(midi, ISynthesizersRegister, synthesizersRegister)

public:
    Sequencer() = default;
    ~Sequencer() override;

    enum Status {
        Stoped = 0,
        Running,
        Error
    };

    Status status() const;

    void loadMIDI(const std::shared_ptr<midi::MidiStream>& stream);

    bool run(float init_sec) override;
    void seek(float sec) override;
    void stop() override;

    float getAudio(float sec, float* buf, unsigned int samples, Context* ctx = nullptr) override;
    bool hasEnded() const override;

    float playbackSpeed() const override;
    void setPlaybackSpeed(float speed) override;

    void setIsTrackMuted(track_t trackIndex, bool mute) override;
    void setTrackVolume(track_t trackIndex, float volume) override;
    void setTrackBalance(track_t trackIndex, float balance) override;

private:

    void process(float sec, Context* ctx);

    void reset();
    tick_t validChunkTick(tick_t fromTick, const Chunks& chunks, tick_t maxDistanceTick) const;
    bool sendEvents(tick_t fromTick, tick_t toTick);

    std::shared_ptr<ISynthesizer> determineSynthesizer(channel_t ch, const std::map<channel_t, std::string>& synthmap) const;
    std::shared_ptr<ISynthesizer> synth(channel_t ch) const;

    void buildTempoMap();
    void setupChannels();

    void setCurrentMSec(uint64_t msec);
    tick_t ticks(uint64_t msec) const;

    bool hasTrack(track_t num) const;

    void requestData(tick_t tick);
    void onChunkReceived(const Chunk& chunk);

    Status m_status = Stoped;

    std::mutex m_dataMutex;
    MidiData m_midiData;
    std::shared_ptr<MidiStream> m_midiStream;

    float m_playSpeed = 1.0;

    msec_t m_prevMSec = 0;
    msec_t m_curMSec = 0;

    bool m_isPlayTickSet = false;
    tick_t m_playTick = 0;    //! NOTE First event tick

    struct TempoItem {
        tempo_t tempo = 500000;
        tick_t startTicks = 0;
        uint64_t startMsec = 0;
        double onetickMsec = 0.0;
    };
    std::map<uint64_t /*msec*/, TempoItem> m_tempoMap;

    struct StreamState {
        std::atomic<bool> requested{ false };
        void reset() { requested = false; }
    };
    StreamState m_streamState;

    struct ChanState {
        bool muted = false;
    };
    std::map<channel_t, ChanState> m_chanStates;

    struct SynthState {
        std::set<channel_t> channels;
        std::shared_ptr<ISynthesizer> synth;
        std::vector<float> buf;
    };
    std::vector<SynthState> m_synthStates;
};
}
}

#endif // MU_MIDI_SEQUENCER_H
