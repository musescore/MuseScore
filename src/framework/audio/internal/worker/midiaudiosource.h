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

#ifndef MU_AUDIO_MIDIPLAYER_H
#define MU_AUDIO_MIDIPLAYER_H

#include <memory>
#include <vector>
#include <map>
#include <cstdint>
#include <functional>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "midi/imidioutport.h"

#include "isynthresolver.h"
#include "track.h"
#include "audiotypes.h"

namespace mu::audio {
class MidiAudioSource : public ITrackAudioInput, public async::Asyncable
{
    INJECT(audio, synth::ISynthResolver, synthResolver)
    INJECT(audio, midi::IMidiOutPort, midiOutPort)

public:
    explicit MidiAudioSource(const TrackId trackId, const midi::MidiData& midiData);
    ~MidiAudioSource() override;

    bool isActive() const override;
    void setIsActive(const bool active) override;

    void setSampleRate(unsigned int sampleRate) override;
    unsigned int audioChannelsCount() const override;
    async::Channel<unsigned int> audioChannelsCountChanged() const override;
    samples_t process(float* buffer, samples_t samplesPerChannel) override;

    void seek(const msecs_t newPositionMsecs) override;
    void applyInputParams(const AudioInputParams& originParams, AudioInputParams& resultParams) override;

private:
    struct EventsBuffer {
        midi::tick_t size = 480 * 4 * 10;

        midi::tick_t currentTick = 0;
        midi::tick_t endTick = 0;

        bool hasEventsForTick(const midi::tick_t tick) const
        {
            return m_eventsMap.find(tick) != m_eventsMap.end();
        }

        std::vector<midi::Event> pop()
        {
            return m_eventsMap.extract(currentTick).mapped();
        }

        void push(midi::Events&& newEvents)
        {
            for (auto& pair : newEvents) {
                for (midi::Event& event : pair.second) {
                    std::vector<midi::Event>& eventsAtTick = m_eventsMap[pair.first];
                    eventsAtTick.push_back(std::move(event));
                }
            }
        }

        bool hasEventsForNextTicks(const midi::tick_t nextTicksCount) const
        {
            if (isEmpty()) {
                return false;
            }

            for (midi::tick_t tick = currentTick; tick <= currentTick + nextTicksCount; ++tick) {
                if (hasEventsForTick(tick)) {
                    return true;
                }
            }

            return false;
        }

        bool isEmpty() const
        {
            return m_eventsMap.empty();
        }

        void reset()
        {
            currentTick = 0;
            endTick = 0;
            m_eventsMap.clear();
        }

    private:
        midi::Events m_eventsMap;
    };

    void handleNextMsecs(const msecs_t nextMsecsNumber);

    midi::tick_t tickFromMsec(const msecs_t msec) const;

    bool hasAnythingToPlayback(const msecs_t nextMsecsNumber) const;
    void handleBackgroundStream(const msecs_t nextMsecsNumber);
    void handleMainStream(const msecs_t nextMsecsNumber);

    void findAndSendNextEvents(EventsBuffer& eventsBuffer, const midi::tick_t nextTicks);
    bool sendEvents(const std::vector<midi::Event>& events);
    void requestNextEvents(const midi::tick_t nextTicksNumber);
    void sendRequestFromTick(const midi::tick_t from);

    void buildTempoMap();
    void setupChannels();

    void invalidateCaches(EventsBuffer& eventsBuffer);

    bool m_hasActiveRequest = false;

    TrackId m_trackId = -1;
    synth::ISynthesizerPtr m_synth = nullptr;
    AudioInputParams m_params;

    midi::MidiStream m_stream;
    midi::MidiMapping m_mapping;

    EventsBuffer m_mainStreamEventsBuffer;
    EventsBuffer m_backgroundStreamEventsBuffer;

    unsigned int m_sampleRate = 0;

    struct TempoItem {
        midi::tempo_t tempo = 500000;
        midi::tick_t startTicks = 0;
        uint64_t startMsec = 0;
        double onetickMsec = 0.0;
    };
    std::map<msecs_t /*msec*/, TempoItem> m_tempoMap = {};
};
}

#endif // MU_AUDIO_MIDIPLAYER_H
