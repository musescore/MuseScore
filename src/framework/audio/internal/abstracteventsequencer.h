/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#ifndef MUSE_AUDIO_ABSTRACTEVENTSEQUENCER_H
#define MUSE_AUDIO_ABSTRACTEVENTSEQUENCER_H

#include <map>
#include <set>

#include "global/async/asyncable.h"
#include "mpe/events.h"

#include "audiosanitizer.h"
#include "../audiotypes.h"

namespace muse::audio {
template<class ... Types>
class AbstractEventSequencer : public async::Asyncable
{
public:
    using EventType = std::variant<Types...>;
    using EventSequence = std::set<EventType>;
    using EventSequenceMap = std::map<msecs_t, EventSequence>;

    typedef typename EventSequenceMap::const_iterator SequenceIterator;
    typedef typename EventSequence::const_iterator EventIterator;

    virtual ~AbstractEventSequencer()
    {
        m_playbackData.mainStream.resetOnReceive(this);
        m_playbackData.offStream.resetOnReceive(this);
    }

    void load(const mpe::PlaybackData& data)
    {
        ONLY_AUDIO_WORKER_THREAD;

        m_playbackData = data;

        m_playbackData.mainStream.onReceive(this, [this](const mpe::PlaybackEventsMap& events,
                                                         const mpe::DynamicLevelLayers& dynamics,
                                                         const mpe::PlaybackParamLayers& params) {
            m_playbackData.originEvents = events;
            m_playbackData.dynamics = dynamics;
            m_playbackData.params = params;
            m_shouldUpdateMainStreamEvents = true;

            if (m_isActive) {
                updateMainStream();
            }
        });

        m_playbackData.offStream.onReceive(this, [this](const mpe::PlaybackEventsMap& events, const mpe::PlaybackParamList& params) {
            updateOffStreamEvents(events, params);
        });

        updateMainStreamEvents(data.originEvents, data.dynamics, data.params);
    }

    const mpe::PlaybackData& playbackData() const
    {
        return m_playbackData;
    }

    void updateMainStream()
    {
        if (m_shouldUpdateMainStreamEvents) {
            updateMainStreamEvents(m_playbackData.originEvents, m_playbackData.dynamics, m_playbackData.params);
            m_shouldUpdateMainStreamEvents = false;
        }
    }

    void setActive(const bool active)
    {
        if (m_isActive == active) {
            return;
        }

        m_isActive = active;

        if (m_isActive) {
            updateMainStream();
        }
    }

    bool isActive() const
    {
        return m_isActive;
    }

    void setPlaybackPosition(const msecs_t newPlaybackPosition)
    {
        ONLY_AUDIO_WORKER_THREAD;

        m_playbackPosition = newPlaybackPosition;
        resetAllIterators();
    }

    msecs_t playbackPosition() const
    {
        ONLY_AUDIO_WORKER_THREAD;

        return m_playbackPosition;
    }

    using OnFlushedCallback = std::function<void ()>;

    void setOnOffStreamFlushed(OnFlushedCallback flushed)
    {
        ONLY_AUDIO_WORKER_THREAD;

        m_onOffStreamFlushed = flushed;
    }

    void setOnMainStreamFlushed(OnFlushedCallback flushed)
    {
        ONLY_AUDIO_WORKER_THREAD;

        m_onMainStreamFlushed = flushed;
    }

    mpe::dynamic_level_t dynamicLevel(const msecs_t position) const
    {
        for (const auto& layer : m_playbackData.dynamics) {
            const mpe::DynamicLevelMap& dynamics = layer.second;
            auto it = muse::findLessOrEqual(dynamics, position);
            if (it != dynamics.end()) {
                return it->second;
            }
        }

        return mpe::dynamicLevelFromType(muse::mpe::DynamicType::Natural);
    }

    EventSequenceMap movePlaybackForward(const msecs_t nextMsecs)
    {
        ONLY_AUDIO_WORKER_THREAD;

        EventSequenceMap result;

        if (!m_isActive) {
            result.emplace(0, EventSequence());
            handleOffStream(result, nextMsecs);
            return result;
        }

        // Empty sequence means to continue the previous sequence
        result.emplace(m_playbackPosition, EventSequence());

        if (m_currentMainSequenceIt == m_mainStreamEvents.cend()) {
            return result;
        }

        m_playbackPosition += nextMsecs;

        handleMainStream(result);
        handleDynamicChanges(result);

        return result;
    }

protected:
    virtual void updateOffStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::PlaybackParamList& params) = 0;
    virtual void updateMainStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelLayers& dynamics,
                                        const mpe::PlaybackParamLayers& params) = 0;

    void resetAllIterators()
    {
        updateMainSequenceIterator();
        updateOffSequenceIterator();
        updateDynamicChangesIterator();
    }

    void updateMainSequenceIterator()
    {
        m_currentMainSequenceIt = m_mainStreamEvents.lower_bound(m_playbackPosition);
    }

    void updateOffSequenceIterator()
    {
        m_currentOffSequenceIt = m_offStreamEvents.cbegin();
    }

    void updateDynamicChangesIterator()
    {
        m_currentDynamicsIt = m_dynamicEvents.lower_bound(m_playbackPosition);
    }

    void handleOffStream(EventSequenceMap& result, const msecs_t nextMsecs)
    {
        if (m_offStreamEvents.empty() || m_currentOffSequenceIt == m_offStreamEvents.cend()) {
            return;
        }

        if (m_currentOffSequenceIt->first <= nextMsecs) {
            while (m_currentOffSequenceIt != m_offStreamEvents.end()
                   && m_currentOffSequenceIt->first <= nextMsecs) {
                result.insert_or_assign(m_currentOffSequenceIt->first, std::move(m_currentOffSequenceIt->second));
                m_currentOffSequenceIt = m_offStreamEvents.erase(m_currentOffSequenceIt);
            }
        } else {
            auto node = m_offStreamEvents.extract(m_currentOffSequenceIt);
            node.key() -= nextMsecs;
            m_offStreamEvents.insert(std::move(node));
            updateOffSequenceIterator();
        }
    }

    void handleMainStream(EventSequenceMap& result)
    {
        while (m_currentMainSequenceIt != m_mainStreamEvents.end()
               && m_currentMainSequenceIt->first <= m_playbackPosition) {
            EventSequence& sequence = result[m_currentMainSequenceIt->first];
            sequence.insert(m_currentMainSequenceIt->second.cbegin(),
                            m_currentMainSequenceIt->second.cend());
            m_currentMainSequenceIt = std::next(m_currentMainSequenceIt);
        }
    }

    void handleDynamicChanges(EventSequenceMap& result)
    {
        if (m_dynamicEvents.empty()) {
            return;
        }

        while (m_currentDynamicsIt != m_dynamicEvents.end()
               && m_currentDynamicsIt->first <= m_playbackPosition) {
            EventSequence& sequence = result[m_currentDynamicsIt->first];
            sequence.insert(m_currentDynamicsIt->second.cbegin(),
                            m_currentDynamicsIt->second.cend());
            m_currentDynamicsIt = std::next(m_currentDynamicsIt);
        }
    }

    mutable msecs_t m_playbackPosition = 0;

    SequenceIterator m_currentMainSequenceIt;
    SequenceIterator m_currentOffSequenceIt;
    SequenceIterator m_currentDynamicsIt;

    EventSequenceMap m_mainStreamEvents;
    EventSequenceMap m_offStreamEvents;
    EventSequenceMap m_dynamicEvents;

    mpe::PlaybackData m_playbackData;

    bool m_isActive = false;

    OnFlushedCallback m_onOffStreamFlushed;
    OnFlushedCallback m_onMainStreamFlushed;

private:
    bool m_shouldUpdateMainStreamEvents = false;
};
}

#endif // MUSE_AUDIO_ABSTRACTEVENTSEQUENCER_H
