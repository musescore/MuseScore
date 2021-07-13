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
#include "playback.h"

#include "log.h"
#include "async/async.h"

#include "internal/audiothread.h"
#include "internal/audiosanitizer.h"

#include "internal/worker/playerhandler.h"
#include "internal/worker/trackshandler.h"
#include "internal/worker/audiooutputhandler.h"
#include "internal/worker/tracksequence.h"

#include "audioerrors.h"

using namespace mu;
using namespace mu::audio;
using namespace mu::async;

void Playback::init()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_playerHandlersPtr = std::make_shared<PlayerHandler>(this);
    m_trackHandlersPtr = std::make_shared<TracksHandler>(this);
    m_audioOutputPtr = std::make_shared<AudioOutputHandler>(this);
}

Promise<TrackSequenceId> Playback::addSequence()
{
    return Promise<TrackSequenceId>([this](Promise<TrackSequenceId>::Resolve resolve, Promise<TrackSequenceId>::Reject /*reject*/) {
        ONLY_AUDIO_WORKER_THREAD;

        TrackSequenceId newId = m_sequences.size();

        m_sequences.emplace(newId, std::make_shared<TrackSequence>(newId));
        m_sequenceAdded.send(newId);

        resolve(std::move(newId));
    }, AudioThread::ID);
}

Promise<TrackSequenceIdList> Playback::sequenceIdList() const
{
    return Promise<TrackSequenceIdList>([this](Promise<TrackSequenceIdList>::Resolve resolve,
                                               Promise<TrackSequenceIdList>::Reject /*reject*/) {
        ONLY_AUDIO_WORKER_THREAD;

        TrackSequenceIdList result(m_sequences.size());

        for (const auto& pair : m_sequences) {
            result.push_back(pair.first);
        }

        resolve(std::move(result));
    }, AudioThread::ID);
}

void Playback::removeSequence(const TrackSequenceId id)
{
    Async::call(this, [this, id]() {
        ONLY_AUDIO_WORKER_THREAD;

        auto search = m_sequences.find(id);

        if (search != m_sequences.end()) {
            m_sequences.erase(search);
        }

        m_sequenceRemoved.send(id);
    }, AudioThread::ID);
}

Channel<TrackSequenceId> Playback::sequenceAdded() const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;

    return m_sequenceAdded;
}

Channel<TrackSequenceId> Playback::sequenceRemoved() const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;

    return m_sequenceRemoved;
}

IPlayerPtr Playback::player() const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;

    return m_playerHandlersPtr;
}

ITracksPtr Playback::tracks() const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;

    return m_trackHandlersPtr;
}

IAudioOutputPtr Playback::audioOutput() const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;

    return m_audioOutputPtr;
}

ITrackSequencePtr Playback::sequence(const TrackSequenceId id) const
{
    ONLY_AUDIO_WORKER_THREAD;

    auto search = m_sequences.find(id);

    if (search != m_sequences.end()) {
        return search->second;
    }

    return nullptr;
}
