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

#ifndef MU_AUDIO_ABSTRACTEVENTSEQUENCER_H
#define MU_AUDIO_ABSTRACTEVENTSEQUENCER_H

#include <map>
#include <set>

#include "async/asyncable.h"
#include "async/channel.h"
#include "async/notification.h"
#include "mpe/events.h"

#include "internal/audiosanitizer.h"
#include "audiotypes.h"

namespace mu::audio {
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
        m_mainStreamChanges.resetOnReceive(this);
        m_offStreamChanges.resetOnReceive(this);
        m_dynamicLevelChanges.resetOnReceive(this);
    }

    void load(const mpe::PlaybackData& data)
    {
        ONLY_AUDIO_WORKER_THREAD;

        m_mainStreamChanges = data.mainStream;
        m_offStreamChanges = data.offStream;
        m_dynamicLevelChanges = data.dynamicLevelChanges;

        m_playbackEventsMap = data.originEvents;
        m_dynamicLevelMap = data.dynamicLevelMap;

        m_offStreamChanges.onReceive(this, [this](const mpe::PlaybackEventsMap& changes) {
            updateOffStreamEvents(changes);
        });

        m_mainStreamChanges.onReceive(this, [this](const mpe::PlaybackEventsMap& changes) {
            m_playbackEventsMap = changes;
            updateMainStreamEvents(changes);
        });

        m_dynamicLevelChanges.onReceive(this, [this](const mpe::DynamicLevelMap& changes) {
            m_dynamicLevelMap = changes;
            updateDynamicChanges(changes);
        });

        updateMainStreamEvents(data.originEvents);
        updateDynamicChanges(data.dynamicLevelMap);
    }

    virtual void updateOffStreamEvents(const mpe::PlaybackEventsMap& changes) = 0;
    virtual void updateMainStreamEvents(const mpe::PlaybackEventsMap& changes) = 0;
    virtual void updateDynamicChanges(const mpe::DynamicLevelMap& changes) = 0;

    async::Notification flushedOffStreamEvents() const
    {
        return m_offStreamFlushed;
    }

    async::Notification flushedMainStreamEvents() const
    {
        return m_mainStreamFlushed;
    }

    void setActive(const bool active)
    {
        m_isActive = active;
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

    mpe::dynamic_level_t dynamicLevel(const msecs_t position) const
    {
        if (m_dynamicLevelMap.empty()) {
            return mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
        }

        if (m_dynamicLevelMap.size() == 1) {
            return m_dynamicLevelMap.cbegin()->second;
        }

        auto upper = m_dynamicLevelMap.upper_bound(position);
        if (upper == m_dynamicLevelMap.cbegin()) {
            return upper->second;
        }

        return std::prev(upper)->second;
    }

    EventSequence eventsToBePlayed(const msecs_t nextMsecs)
    {
        ONLY_AUDIO_WORKER_THREAD;

        EventSequence result;

        result.clear();

        if (!m_isActive) {
            handleOffStream(result, nextMsecs);
            return result;
        }

        if (m_currentMainSequenceIt == m_mainStreamEvents.cend()) {
            return result;
        }

        m_playbackPosition += nextMsecs;

        handleMainStream(result);
        handleDynamicChanges(result);

        return result;
    }

protected:
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

    void handleOffStream(EventSequence& result, const msecs_t nextMsecs)
    {
        if (m_offStreamEvents.empty() || m_currentOffSequenceIt == m_offStreamEvents.cend()) {
            return;
        }

        if (m_currentOffSequenceIt->first <= nextMsecs) {
            result = m_currentOffSequenceIt->second;
            m_currentOffSequenceIt = m_offStreamEvents.erase(m_currentOffSequenceIt);
        } else {
            auto node = m_offStreamEvents.extract(m_currentOffSequenceIt);
            node.key() -= nextMsecs;
            m_offStreamEvents.insert(std::move(node));
            updateOffSequenceIterator();
        }
    }

    void handleMainStream(EventSequence& result)
    {
        if (m_currentMainSequenceIt->first <= m_playbackPosition) {
            result.insert(m_currentMainSequenceIt->second.cbegin(),
                          m_currentMainSequenceIt->second.cend());

            m_currentMainSequenceIt = std::next(m_currentMainSequenceIt);
        }
    }

    void handleDynamicChanges(EventSequence& result)
    {
        if (m_dynamicEvents.empty() || m_currentDynamicsIt == m_dynamicEvents.cend()) {
            return;
        }

        if (m_currentDynamicsIt->first <= m_playbackPosition) {
            result.insert(m_currentDynamicsIt->second.cbegin(),
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

    mpe::DynamicLevelMap m_dynamicLevelMap;
    mpe::PlaybackEventsMap m_playbackEventsMap;

    async::Notification m_offStreamFlushed;
    async::Notification m_mainStreamFlushed;

    bool m_isActive = false;

    mpe::PlaybackEventsChanges m_mainStreamChanges;
    mpe::PlaybackEventsChanges m_offStreamChanges;
    mpe::DynamicLevelChanges m_dynamicLevelChanges;
};
}

#endif // MU_AUDIO_ABSTRACTEVENTSEQUENCER_H
