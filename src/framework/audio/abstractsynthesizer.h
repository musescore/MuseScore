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

#ifndef MU_AUDIO_ISYNTHESIZER_H
#define MU_AUDIO_ISYNTHESIZER_H

#include "async/channel.h"
#include "async/asyncable.h"
#include "mpe/events.h"

#include "synthtypes.h"
#include "audiotypes.h"
#include "iaudiosource.h"

namespace mu::audio::synth {
class AbstractSynthesizer : public IAudioSource, public async::Asyncable
{
public:
    AbstractSynthesizer(const audio::AudioInputParams& params);
    virtual ~AbstractSynthesizer();

    virtual std::string name() const = 0;
    virtual AudioSourceType type() const = 0;
    const audio::AudioInputParams& params() const;
    async::Channel<audio::AudioInputParams> paramsChanged() const;

    msecs_t playbackPosition() const;
    void setPlaybackPosition(const msecs_t newPosition);

    void setup(const mpe::PlaybackData& playbackData);
    virtual void flushSound() = 0;
    virtual void revokePlayingNotes();

    virtual bool isValid() const = 0;
    virtual bool isActive() const;
    virtual void setIsActive(bool arg);

    static constexpr float DAMPER_FACTOR = 3.0;
    static constexpr msecs_t MIN_NOTE_LENGTH = 50;

protected:
    using EventsMapIterator = mpe::PlaybackEventsMap::const_iterator;
    using EventsMapIteratorList = std::vector<EventsMapIterator>;

    struct EventsBuffer {
        msecs_t from = 0;
        msecs_t to = 0;

        const mpe::PlaybackEventsMap& events() const
        {
            return m_events;
        }

        void load(const mpe::PlaybackEventsMap& events)
        {
            m_events = events;
            updateBoundaries();
        }

        void insert(const mpe::PlaybackEventsMap& events)
        {
            for (const auto& pair : events) {
                m_events[pair.first] = pair.second;
            }

            updateBoundaries();
        }

        EventsMapIteratorList findEventsRange(const msecs_t rangeFrom, const msecs_t rangeTo) const
        {
            if (m_events.empty()) {
                static EventsMapIteratorList empty;
                return empty;
            }

            auto firstNotLess = m_events.lower_bound(rangeFrom);

            if (firstNotLess == m_events.begin()) {
                return { firstNotLess };
            }

            auto firstLess = std::prev(firstNotLess);

            if (firstNotLess->first > rangeTo || firstNotLess == m_events.cend()) {
                return { firstLess };
            }

            return { firstLess, firstNotLess };
        }

        void clear()
        {
            m_events.clear();
            updateBoundaries();
        }

        bool empty() const
        {
            return m_events.empty();
        }

    private:

        void updateBoundaries()
        {
            if (empty()) {
                from = 0;
                to = 0;
                return;
            }

            from = m_events.begin()->first;

            auto lastEvents = m_events.rbegin();
            to = lastEvents->first;

            for (const mpe::PlaybackEvent& event : lastEvents->second) {
                if (!std::holds_alternative<mpe::NoteEvent>(event)) {
                    continue;
                }

                const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

                mpe::duration_t dampedDuration = noteEvent.arrangementCtx().actualDuration * DAMPER_FACTOR;
                dampedDuration = std::max(dampedDuration, MIN_NOTE_LENGTH);
                to = std::max(noteEvent.arrangementCtx().actualTimestamp + dampedDuration, to);
            }
        }

        mpe::PlaybackEventsMap m_events;
    };

    virtual void setupSound(const mpe::PlaybackSetupData& setupData) = 0;
    virtual void setupEvents(const mpe::PlaybackData& playbackData);

    msecs_t samplesToMsecs(const samples_t samplesPerChannel, const samples_t sampleRate) const;

    msecs_t m_playbackPosition = 0;

    mpe::PlaybackSetupData m_setupData;
    EventsBuffer m_mainStreamEvents;
    EventsBuffer m_offStreamEvents;

    mpe::PlaybackEventsChanges m_mainStreamChanges;
    mpe::PlaybackEventsChanges m_offStreamChanges;

    audio::AudioInputParams m_params;
    async::Channel<audio::AudioInputParams> m_paramsChanges;

    bool m_isActive = false;
    samples_t m_sampleRate = 0;
};

using ISynthesizerPtr = std::shared_ptr<AbstractSynthesizer>;
}

#endif // MU_AUDIO_ISYNTHESIZER_H
