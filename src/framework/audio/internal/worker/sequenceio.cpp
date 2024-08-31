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

#include "sequenceio.h"

#include "internal/audiosanitizer.h"
#include "audioerrors.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::async;

SequenceIO::SequenceIO(IGetTracks* getTracks)
    : m_getTracks(getTracks)
{
    m_getTracks->trackAboutToBeAdded().onReceive(this, [this](TrackPtr trackPtr) {
        if (!trackPtr) {
            return;
        }

        TrackId id = trackPtr->id;

        trackPtr->inputParamsChanged().onReceive(this, [this, id](const AudioInputParams& params) {
            m_inputParamsChanged.send(id, params);
        });

        trackPtr->outputParamsChanged().onReceive(this, [this, id](const AudioOutputParams& params) {
            m_outputParamsChanged.send(id, params);
        });
    });

    m_getTracks->trackAboutToBeRemoved().onReceive(this, [this](TrackPtr trackPtr) {
        if (!trackPtr) {
            return;
        }

        trackPtr->inputParamsChanged().resetOnReceive(this);
        trackPtr->outputParamsChanged().resetOnReceive(this);
    });
}

bool SequenceIO::isHasTrack(const TrackId id) const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_getTracks) {
        return false;
    }

    TrackPtr track = m_getTracks->track(id);
    return track != nullptr;
}

RetVal<AudioInputParams> SequenceIO::inputParams(const TrackId id) const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_getTracks) {
        return {};
    }

    RetVal<AudioInputParams> result;

    TrackPtr track = m_getTracks->track(id);

    if (track) {
        return result.make_ok(track->inputParams());
    }

    return make_ret(Err::InvalidTrackId);
}

RetVal<AudioOutputParams> SequenceIO::outputParams(const TrackId id) const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_getTracks) {
        return {};
    }

    RetVal<AudioOutputParams> result;

    TrackPtr track = m_getTracks->track(id);

    if (track) {
        return result.make_ok(track->outputParams());
    }

    return make_ret(Err::InvalidTrackId);
}

void SequenceIO::setInputParams(const TrackId id, const AudioInputParams& params)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_getTracks) {
        return;
    }

    TrackPtr track = m_getTracks->track(id);
    track->setInputParams(params);
}

void SequenceIO::setOutputParams(const TrackId id, const AudioOutputParams& params)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_getTracks) {
        return;
    }

    TrackPtr track = m_getTracks->track(id);
    track->setOutputParams(params);
}

Channel<TrackId, AudioInputParams> SequenceIO::inputParamsChanged() const
{
    return m_inputParamsChanged;
}

Channel<TrackId, AudioOutputParams> SequenceIO::outputParamsChanged() const
{
    return m_outputParamsChanged;
}

AudioSignalChanges SequenceIO::audioSignalChanges(const TrackId id) const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_getTracks) {
        return {};
    }

    TrackPtr track = m_getTracks->track(id);
    IF_ASSERT_FAILED(track) {
        return AudioSignalChanges();
    }

    return track->outputHandler->audioSignalChanges();
}
