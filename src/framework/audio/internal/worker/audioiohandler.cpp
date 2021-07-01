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

#include "audioiohandler.h"

#include "log.h"
#include "async/async.h"

#include "internal/audiosanitizer.h"
#include "internal/audiothread.h"
#include "audioerrors.h"

using namespace mu::audio;
using namespace mu::async;

AudioIOHandler::AudioIOHandler(IGetTrackSequence* getSequence)
    : m_getSequence(getSequence)
{
}

Promise<AudioInputParams> AudioIOHandler::inputParams(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    return Promise<AudioInputParams>([this, sequenceId, trackId](Promise<AudioInputParams>::Resolve resolve,
                                                                 Promise<AudioInputParams>::Reject reject) {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (!s) {
            reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }

        RetVal<AudioInputParams> result = s->audioIO()->inputParams(trackId);

        if (!result.ret) {
            reject(result.ret.code(), result.ret.text());
        }

        resolve(result.val);
    }, AudioThread::ID);
}

void AudioIOHandler::setInputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioInputParams& params)
{
    Async::call(this, [this, sequenceId, trackId, params]() {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (s) {
            s->audioIO()->setInputParams(trackId, params);
        }
    }, AudioThread::ID);
}

Channel<TrackSequenceId, TrackId, AudioInputParams> AudioIOHandler::inputParamsChanged() const
{
    return m_inputParamsChanged;
}

Promise<AudioOutputParams> AudioIOHandler::outputParams(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    return Promise<AudioOutputParams>([this, sequenceId, trackId](Promise<AudioOutputParams>::Resolve resolve,
                                                                  Promise<AudioOutputParams>::Reject reject) {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (!s) {
            reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }

        RetVal<AudioOutputParams> result = s->audioIO()->outputParams(trackId);

        if (!result.ret) {
            reject(result.ret.code(), result.ret.text());
        }

        resolve(result.val);
    }, AudioThread::ID);
}

void AudioIOHandler::setOutputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioOutputParams& params)
{
    Async::call(this, [this, sequenceId, trackId, params]() {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (s) {
            s->audioIO()->setOutputParams(trackId, params);
        }
    }, AudioThread::ID);
}

Channel<TrackSequenceId, TrackId, AudioOutputParams> AudioIOHandler::outputParamsChanged() const
{
    return m_outputParamsChanged;
}

Promise<AudioOutputParams> AudioIOHandler::globalOutputParams() const
{
    return Promise<AudioOutputParams>([this](Promise<AudioOutputParams>::Resolve resolve,
                                             Promise<AudioOutputParams>::Reject /*reject*/) {
        ONLY_AUDIO_WORKER_THREAD;

        resolve(mixer()->masterOutputParams());
    }, AudioThread::ID);
}

void AudioIOHandler::setGlobalOutputParams(const AudioOutputParams& params)
{
    Async::call(this, [this, params]() {
        ONLY_AUDIO_WORKER_THREAD;

        mixer()->setMasterOutputParams(params);
    }, AudioThread::ID);
}

Channel<AudioOutputParams> AudioIOHandler::globalOutputParamsChanged() const
{
    return mixer()->masterOutputParamsChanged();
}

Channel<audioch_t, float> AudioIOHandler::masterSignalAmplitudeChanged() const
{
    return mixer()->masterSignalAmplitudeRmsChanged();
}

Channel<audioch_t, volume_dbfs_t> AudioIOHandler::masterVolumePressureChanged() const
{
    return mixer()->masterVolumePressureDbfsChanged();
}

ITrackSequencePtr AudioIOHandler::sequence(const TrackSequenceId id) const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_getSequence) {
        return nullptr;
    }

    ITrackSequencePtr s = m_getSequence->sequence(id);
    ensureSubscriptions(s);

    return s;
}

void AudioIOHandler::ensureSubscriptions(const ITrackSequencePtr s) const
{
    ONLY_AUDIO_WORKER_THREAD;

    if (!s) {
        return;
    }

    if (!s->audioIO()->inputParamsChanged().isConnected()) {
        s->audioIO()->inputParamsChanged().onReceive(this, [this, s](const TrackId trackId, const AudioInputParams& params) {
            m_inputParamsChanged.send(s->id(), trackId, params);
        });
    }

    if (!s->audioIO()->outputParamsChanged().isConnected()) {
        s->audioIO()->outputParamsChanged().onReceive(this, [this, s](const TrackId trackId, const AudioOutputParams& params) {
            m_outputParamsChanged.send(s->id(), trackId, params);
        });
    }
}
