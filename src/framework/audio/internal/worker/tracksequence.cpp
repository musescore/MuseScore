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

#include "tracksequence.h"

#include "log.h"

#include "internal/audiosanitizer.h"
#include "internal/audiothread.h"
#include "clock.h"
#include "eventaudiosource.h"
#include "sequenceplayer.h"
#include "sequenceio.h"
#include "audioengine.h"
#include "audioerrors.h"

using namespace mu;
using namespace mu::async;
using namespace mu::audio;

TrackSequence::TrackSequence(const TrackSequenceId id)
    : m_id(id)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_clock = std::make_shared<Clock>();
    m_player = std::make_shared<SequencePlayer>(this, m_clock);
    m_audioIO = std::make_shared<SequenceIO>(this);

    mixer()->addClock(m_clock);
}

TrackSequence::~TrackSequence()
{
    ONLY_AUDIO_WORKER_THREAD;

    mixer()->removeClock(m_clock);

    removeAllTracks();
}

TrackSequenceId TrackSequence::id() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_id;
}

RetVal2<TrackId, AudioParams> TrackSequence::addTrack(const std::string& trackName, const mpe::PlaybackData& playbackData,
                                                      const AudioParams& requiredParams)
{
    ONLY_AUDIO_WORKER_THREAD;

    RetVal2<TrackId, AudioParams> result;
    result.val1 = -1;

    IF_ASSERT_FAILED(mixer()) {
        result.ret = make_ret(Err::Undefined);
        return result;
    }

    if (!playbackData.setupData.isValid()) {
        result.ret = make_ret(Err::InvalidSetupData);
        return result;
    }

    TrackId newId = newTrackId();

    EventTrackPtr trackPtr = std::make_shared<EventTrack>();
    trackPtr->id = newId;
    trackPtr->name = trackName;
    trackPtr->setPlaybackData(playbackData);
    trackPtr->inputHandler = std::make_shared<EventAudioSource>(newId, playbackData);
    trackPtr->outputHandler = mixer()->addChannel(newId, trackPtr->inputHandler).val;
    trackPtr->setInputParams(requiredParams.in);
    trackPtr->setOutputParams(requiredParams.out);

    m_trackAboutToBeAdded.send(trackPtr);
    m_tracks.emplace(newId, trackPtr);
    m_trackAdded.send(newId);

    result.ret = make_ret(Err::NoError);
    result.val1 = newId;
    result.val2 = { trackPtr->inputParams(), trackPtr->outputParams() };

    return result;
}

RetVal2<TrackId, AudioParams> TrackSequence::addTrack(const std::string& trackName, io::IODevice* device, const AudioParams& requiredParams)
{
    ONLY_AUDIO_WORKER_THREAD;

    NOT_IMPLEMENTED;

    RetVal2<TrackId, AudioParams> result;

    if (!device) {
        result.ret = make_ret(Err::InvalidAudioFilePath);
        result.val1 = -1;
        return result;
    }

    TrackId newId = newTrackId();

    SoundTrackPtr trackPtr = std::make_shared<SoundTrack>();
    trackPtr->id = newId;
    trackPtr->name = trackName;
    trackPtr->setPlaybackData(device);
    trackPtr->setInputParams(requiredParams.in);
    trackPtr->setOutputParams(requiredParams.out);
    //TODO create AudioSource and MixerChannel

    m_trackAboutToBeAdded.send(trackPtr);
    m_tracks.emplace(newId, trackPtr);
    m_trackAdded.send(newId);

    result.ret = make_ret(Err::NoError);
    result.val1 = newId;
    result.val2 = { trackPtr->inputParams(), trackPtr->outputParams() };

    return result;
}

TrackName TrackSequence::trackName(const TrackId id) const
{
    TrackPtr trackPtr = track(id);

    if (trackPtr) {
        return trackPtr->name;
    }

    static TrackName emptyName;
    return emptyName;
}

TrackIdList TrackSequence::trackIdList() const
{
    ONLY_AUDIO_WORKER_THREAD;

    TrackIdList result;

    for (const auto& pair : m_tracks) {
        result.push_back(pair.first);
    }

    return result;
}

Ret TrackSequence::removeTrack(const TrackId id)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(mixer()) {
        return make_ret(Err::Undefined);
    }

    auto search = m_tracks.find(id);

    if (search != m_tracks.end() && search->second) {
        m_trackAboutToBeRemoved.send(search->second);
        mixer()->removeChannel(id);
        m_tracks.erase(id);
        m_trackRemoved.send(id);
        return true;
    }

    return make_ret(Err::InvalidTrackId);
}

void TrackSequence::removeAllTracks()
{
    ONLY_AUDIO_WORKER_THREAD;

    for (const TrackId& id : trackIdList()) {
        removeTrack(id);
    }
}

Channel<TrackId> TrackSequence::trackAdded() const
{
    return m_trackAdded;
}

Channel<TrackId> TrackSequence::trackRemoved() const
{
    return m_trackRemoved;
}

ISequencePlayerPtr TrackSequence::player() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_player;
}

ISequenceIOPtr TrackSequence::audioIO() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_audioIO;
}

TrackPtr TrackSequence::track(const TrackId id) const
{
    ONLY_AUDIO_WORKER_THREAD;

    auto search = m_tracks.find(id);

    if (search != m_tracks.end()) {
        return search->second;
    }

    return nullptr;
}

TracksMap TrackSequence::allTracks() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_tracks;
}

Channel<TrackPtr> TrackSequence::trackAboutToBeAdded() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_trackAboutToBeAdded;
}

Channel<TrackPtr> TrackSequence::trackAboutToBeRemoved() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_trackAboutToBeRemoved;
}

TrackId TrackSequence::newTrackId() const
{
    if (m_tracks.empty()) {
        return static_cast<TrackId>(m_tracks.size());
    }

    auto last = m_tracks.rbegin();

    return last->first + 1;
}

std::shared_ptr<Mixer> TrackSequence::mixer() const
{
    return AudioEngine::instance()->mixer();
}
