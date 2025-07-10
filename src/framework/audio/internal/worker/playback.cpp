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

#include <utility>

#include "global/async/async.h"

#include "internal/audiothread.h"
#include "internal/audiosanitizer.h"

#include "internal/worker/player.h"
#include "internal/worker/trackshandler.h"
#include "internal/worker/audiooutputhandler.h"
#include "internal/worker/tracksequence.h"

#include "audioerrors.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::async;

void Playback::init()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_trackHandlersPtr = std::make_shared<TracksHandler>(this, iocContext());
    m_audioOutputPtr = std::make_shared<AudioOutputHandler>(this, iocContext());
}

void Playback::deinit()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_sequences.clear();

    m_trackHandlersPtr = nullptr;
    m_audioOutputPtr = nullptr;

    disconnectAll();
}

bool Playback::isInited() const
{
    return m_trackHandlersPtr != nullptr;
}

Promise<TrackSequenceId> Playback::addSequence()
{
    return Promise<TrackSequenceId>([this](auto resolve, auto /*reject*/) {
        ONLY_AUDIO_WORKER_THREAD;

        TrackSequenceId newId = static_cast<TrackSequenceId>(m_sequences.size());

        m_sequences.emplace(newId, std::make_shared<TrackSequence>(newId, iocContext()));
        m_sequenceAdded.send(newId);

        return resolve(std::move(newId));
    }, AudioThread::ID);
}

Promise<TrackSequenceIdList> Playback::sequenceIdList() const
{
    return Promise<TrackSequenceIdList>([this](auto resolve, auto /*reject*/) {
        ONLY_AUDIO_WORKER_THREAD;

        TrackSequenceIdList result;

        for (const auto& pair : m_sequences) {
            result.push_back(pair.first);
        }

        return resolve(std::move(result));
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

IPlayerPtr Playback::player(const TrackSequenceId id) const
{
    std::shared_ptr<Player> p = std::make_shared<Player>(this, id);
    p->init();
    return p;
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

// ITracks
async::Promise<TrackIdList> Playback::trackIdList(const TrackSequenceId sequenceId) const
{
    return m_trackHandlersPtr->trackIdList(sequenceId);
}

async::Promise<TrackName> Playback::trackName(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    return m_trackHandlersPtr->trackName(sequenceId, trackId);
}

async::Promise<TrackId, AudioParams> Playback::addTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                        io::IODevice* playbackData, AudioParams&& params)
{
    return m_trackHandlersPtr->addTrack(sequenceId, trackName, playbackData, std::move(params));
}

async::Promise<TrackId, AudioParams> Playback::addTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                        const mpe::PlaybackData& playbackData, AudioParams&& params)
{
    return m_trackHandlersPtr->addTrack(sequenceId, trackName, playbackData, std::move(params));
}

async::Promise<TrackId, AudioOutputParams> Playback::addAuxTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                                 const AudioOutputParams& outputParams)
{
    return m_trackHandlersPtr->addAuxTrack(sequenceId, trackName, outputParams);
}

void Playback::removeTrack(const TrackSequenceId sequenceId, const TrackId trackId)
{
    m_trackHandlersPtr->removeTrack(sequenceId, trackId);
}

void Playback::removeAllTracks(const TrackSequenceId sequenceId)
{
    m_trackHandlersPtr->removeAllTracks(sequenceId);
}

async::Channel<TrackSequenceId, TrackId> Playback::trackAdded() const
{
    return m_trackHandlersPtr->trackAdded();
}

async::Channel<TrackSequenceId, TrackId> Playback::trackRemoved() const
{
    return m_trackHandlersPtr->trackRemoved();
}

async::Promise<AudioResourceMetaList> Playback::availableInputResources() const
{
    return m_trackHandlersPtr->availableInputResources();
}

async::Promise<SoundPresetList> Playback::availableSoundPresets(const AudioResourceMeta& resourceMeta) const
{
    return m_trackHandlersPtr->availableSoundPresets(resourceMeta);
}

async::Promise<AudioInputParams> Playback::inputParams(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    return m_trackHandlersPtr->inputParams(sequenceId, trackId);
}

void Playback::setInputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioInputParams& params)
{
    m_trackHandlersPtr->setInputParams(sequenceId, trackId, params);
}

async::Channel<TrackSequenceId, TrackId, AudioInputParams> Playback::inputParamsChanged() const
{
    return m_trackHandlersPtr->inputParamsChanged();
}

void Playback::clearSources()
{
    m_trackHandlersPtr->clearSources();
}

// ===
