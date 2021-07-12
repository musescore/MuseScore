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

#include "log.h"

#include "internal/audiosanitizer.h"
#include "audioerrors.h"

using namespace mu;
using namespace mu::audio;
using namespace mu::async;

SequenceIO::SequenceIO(IGetTracks* getTracks)
    : m_getTracks(getTracks)
{
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

    if (track && track->setInputParams(params)) {
        m_inputParamsChanged.send(id, params);
    }
}

void SequenceIO::setOutputParams(const TrackId id, const AudioOutputParams& params)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_getTracks) {
        return;
    }

    TrackPtr track = m_getTracks->track(id);

    if (track && track->setOutputParams(params)) {
        m_outputParamsChanged.send(id, params);
    }
}

Channel<TrackId, AudioInputParams> SequenceIO::inputParamsChanged() const
{
    return m_inputParamsChanged;
}

Channel<TrackId, AudioOutputParams> SequenceIO::outputParamsChanged() const
{
    return m_outputParamsChanged;
}

Channel<audioch_t, float> SequenceIO::signalAmplitudeChanged(const TrackId id) const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_getTracks) {
        return {};
    }

    TrackPtr track = m_getTracks->track(id);

    return track->mixerChannel->signalAmplitudeRmsChanged();
}

Channel<audioch_t, volume_dbfs_t> SequenceIO::volumePressureChanged(const TrackId id) const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_getTracks) {
        return {};
    }

    TrackPtr track = m_getTracks->track(id);

    return track->mixerChannel->volumePressureDbfsChanged();
}
