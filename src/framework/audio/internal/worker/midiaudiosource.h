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

#include "isynthfactory.h"
#include "audiotypes.h"

namespace mu::audio {
class MidiAudioSource : public IAudioSource, public async::Asyncable
{
    INJECT(audio, synth::ISynthFactory, synthFactory)
    INJECT(audio, midi::IMidiOutPort, midiOutPort)

public:
    explicit MidiAudioSource(const midi::MidiData& midiData, async::Channel<AudioInputParams> inputParamsChanged);
    ~MidiAudioSource() = default;

    bool isActive() const override;
    void setIsActive(const bool active) override;

    void setSampleRate(unsigned int sampleRate) override;
    unsigned int audioChannelsCount() const override;
    async::Channel<unsigned int> audioChannelsCountChanged() const override;
    void process(float* buffer, unsigned int sampleCount) override;

    void seek(const msecs_t newPositionMsecs) override;

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

    void handleBackgroundStream(const msecs_t nextMsecsNumber);
    void handleMainStream(const msecs_t nextMsecsNumber);

    void findAndSendNextEvents(EventsBuffer& eventsBuffer, const midi::tick_t nextTicks);
    bool sendEvents(const std::vector<midi::Event>& events);
    void requestNextEvents(const midi::tick_t nextTicksNumber);

    void resolveSynth(const synth::SynthName& synthName);
    void buildTempoMap();
    void setupChannels();

    void invalidateCaches(EventsBuffer& eventsBuffer);

    bool m_hasActiveRequest = false;

    synth::ISynthesizerPtr m_synth = nullptr;

    midi::MidiStream m_stream;
    midi::MidiMapping m_mapping;

    EventsBuffer m_mainStreamEvents;
    EventsBuffer m_backgroundStreamEvents;

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
