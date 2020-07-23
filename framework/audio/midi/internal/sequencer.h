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

#ifndef MU_AUDIO_SEQUENCER_H
#define MU_AUDIO_SEQUENCER_H

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
#include "../isynthesizer.h"
#include "async/asyncable.h"

namespace mu {
namespace audio {
namespace midi {
class ISynthesizer;
class Sequencer : public ISequencer, public async::Asyncable
{
    INJECT(midi, ISynthesizer, synth)

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
    void init(float samplerate, float gain = 1);

    void changeGain(float gain);

    bool run(float init_sec) override;
    void seek(float sec) override;
    void stop() override;

    float getAudio(float sec, float* buf, unsigned int len) override;
    bool hasEnded() const override;

    float playbackPosition() const;

    float playbackSpeed() const override;
    void setPlaybackSpeed(float speed) override;

    void setIsTrackMuted(uint16_t trackIndex, bool mute) override;
    void setTrackVolume(uint16_t trackIndex, float volume) override;
    void setTrackBalance(uint16_t trackIndex, float balance) override;

private:

    void process(float sec);

    void reset();
    uint64_t maxTicks(const std::vector<Track>& tracks) const;
    bool channelEOT(const Channel& chan) const;
    bool sendEvents(uint32_t curTicks);
    bool sendChanEvents(const Channel& chan, uint32_t ticks);

    void buildTempoMap();

    uint32_t ticks(uint64_t msec) const;

    bool hasTrack(uint16_t num) const;

    bool doRun();
    void doStop();
    void doSeek(uint64_t seekMsec);
    void doSeekTracks(uint32_t seekTicks, const std::vector<Track>& tracks);
    void doSeekChan(uint32_t seekTicks, const Channel& c);

    void requestData(uint32_t tick);
    void onDataReceived(const MidiData& data);
    void onStreamClosed();

    struct TempoItem {
        uint32_t tempo = 500000;
        uint32_t startTicks = 0;
        uint64_t startMsec = 0;
        double onetickMsec = 0.0;
    };
    std::map<uint64_t /*msec*/, TempoItem> m_tempoMap;

    Status m_status = Stoped;
    bool m_internalRunning = false;

    MidiData m_midiData;
    std::shared_ptr<MidiStream> m_midiStream;

    struct StreamState {
        bool requested = false;
        bool closed = false;
    };

    StreamState m_streamState;

    double m_oneTickMsec = 1;

    float m_sampleRate = 44100.0f;
    float m_playSpeed = 1.0;

    uint64_t m_lastTimeMsec = 0;
    uint64_t m_curMsec = 0;
    uint64_t m_seekMsec = 0;

    struct ChanState {
        bool muted = false;
        size_t eventIndex = 0;
    };
    mutable std::map<uint16_t, ChanState> m_chanStates;
};
}
}
}

#endif // MU_AUDIO_SEQUENCER_H
