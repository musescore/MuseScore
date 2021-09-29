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

#include "audiooutputhandler.h"

#include "log.h"
#include "async/async.h"

#include "internal/audiosanitizer.h"
#include "internal/audiothread.h"
#include "internal/worker/audioengine.h"
#include "audioerrors.h"

using namespace mu::audio;
using namespace mu::async;

AudioOutputHandler::AudioOutputHandler(IGetTrackSequence* getSequence)
    : m_getSequence(getSequence)
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;

    Async::call(this, [this]() {
        ensureMixerSubscriptions();
    }, AudioThread::ID);
}

Promise<AudioOutputParams> AudioOutputHandler::outputParams(const TrackSequenceId sequenceId, const TrackId trackId) const
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

void AudioOutputHandler::setOutputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioOutputParams& params)
{
    Async::call(this, [this, sequenceId, trackId, params]() {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (s) {
            s->audioIO()->setOutputParams(trackId, params);
        }
    }, AudioThread::ID);
}

Channel<TrackSequenceId, TrackId, AudioOutputParams> AudioOutputHandler::outputParamsChanged() const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;

    return m_outputParamsChanged;
}

Promise<AudioOutputParams> AudioOutputHandler::masterOutputParams() const
{
    return Promise<AudioOutputParams>([this](Promise<AudioOutputParams>::Resolve resolve,
                                             Promise<AudioOutputParams>::Reject reject) {
        ONLY_AUDIO_WORKER_THREAD;

        IF_ASSERT_FAILED(mixer()) {
            reject(static_cast<int>(Err::Undefined), "undefined reference to a mixer");
        }

        resolve(mixer()->masterOutputParams());
    }, AudioThread::ID);
}

void AudioOutputHandler::setMasterOutputParams(const AudioOutputParams& params)
{
    Async::call(this, [this, params]() {
        ONLY_AUDIO_WORKER_THREAD;

        IF_ASSERT_FAILED(mixer()) {
            return;
        }

        mixer()->setMasterOutputParams(params);
    }, AudioThread::ID);
}

Channel<AudioOutputParams> AudioOutputHandler::masterOutputParamsChanged() const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;

    return m_masterOutputParamsChanged;
}

Promise<AudioResourceMetaList> AudioOutputHandler::availableOutputResources() const
{
    return Promise<AudioResourceMetaList>([this](Promise<AudioResourceMetaList>::Resolve resolve,
                                                 Promise<AudioResourceMetaList>::Reject /*reject*/) {
        ONLY_AUDIO_WORKER_THREAD;

        resolve(fxResolver()->resolveAvailableResources());
    }, AudioThread::ID);
}

Promise<AudioSignalChanges> AudioOutputHandler::signalChanges(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    return Promise<AudioSignalChanges>([this, sequenceId, trackId](Promise<AudioSignalChanges>::Resolve resolve,
                                                                   Promise<AudioSignalChanges>::Reject reject) {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (!s) {
            reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
            return;
        }

        if (!s->audioIO()->isHasTrack(trackId)) {
            reject(static_cast<int>(Err::InvalidTrackId), "no track");
            return;
        }

        resolve(s->audioIO()->audioSignalChanges(trackId));
    }, AudioThread::ID);
}

Promise<AudioSignalChanges> AudioOutputHandler::masterSignalChanges() const
{
    return Promise<AudioSignalChanges>([this](Promise<AudioSignalChanges>::Resolve resolve,
                                              Promise<AudioSignalChanges>::Reject reject) {
        ONLY_AUDIO_WORKER_THREAD;

        IF_ASSERT_FAILED(mixer()) {
            reject(static_cast<int>(Err::Undefined), "undefined reference to a mixer");
        }

        resolve(mixer()->masterAudioSignalChanges());
    }, AudioThread::ID);
}

std::shared_ptr<Mixer> AudioOutputHandler::mixer() const
{
    return AudioEngine::instance()->mixer();
}

ITrackSequencePtr AudioOutputHandler::sequence(const TrackSequenceId id) const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_getSequence) {
        return nullptr;
    }

    ITrackSequencePtr s = m_getSequence->sequence(id);
    ensureSeqSubscriptions(s);

    return s;
}

void AudioOutputHandler::ensureSeqSubscriptions(const ITrackSequencePtr s) const
{
    ONLY_AUDIO_WORKER_THREAD;

    if (!s) {
        return;
    }

    TrackSequenceId sequenceId = s->id();

    if (!s->audioIO()->outputParamsChanged().isConnected()) {
        s->audioIO()->outputParamsChanged().onReceive(this, [this, sequenceId](const TrackId trackId, const AudioOutputParams& params) {
            m_outputParamsChanged.send(sequenceId, trackId, params);
        });
    }
}

void AudioOutputHandler::ensureMixerSubscriptions() const
{
    ONLY_AUDIO_WORKER_THREAD;

    if (!mixer()) {
        return;
    }

    if (!mixer()->masterOutputParamsChanged().isConnected()) {
        mixer()->masterOutputParamsChanged().onReceive(this, [this](const AudioOutputParams& params) {
            m_masterOutputParamsChanged.send(params);
        });
    }
}
