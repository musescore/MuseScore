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

#include "global/async/async.h"
#include "global/containers.h"

#include "internal/audiosanitizer.h"
#include "internal/audiothread.h"
#include "internal/worker/audioengine.h"
#include "audioerrors.h"

#include "muse_framework_config.h"
#ifdef MUSE_MODULE_AUDIO_EXPORT
#include "internal/soundtracks/soundtrackwriter.h"
#endif

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::async;

#ifdef MUSE_MODULE_AUDIO_EXPORT
using namespace muse::audio::soundtrack;
#endif

AudioOutputHandler::AudioOutputHandler(IGetTrackSequence* getSequence, const modularity::ContextPtr& iocCtx)
    : Injectable(iocCtx), m_getSequence(getSequence)
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;

    Async::call(this, [this]() {
        ensureMixerSubscriptions();
    }, AudioThread::ID);
}

Promise<AudioOutputParams> AudioOutputHandler::outputParams(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    return Promise<AudioOutputParams>([this, sequenceId, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (!s) {
            return reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }

        RetVal<AudioOutputParams> result = s->audioIO()->outputParams(trackId);

        if (!result.ret) {
            return reject(result.ret.code(), result.ret.text());
        }

        return resolve(result.val);
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
    return Promise<AudioOutputParams>([this](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        IF_ASSERT_FAILED(mixer()) {
            return reject(static_cast<int>(Err::Undefined), "undefined reference to a mixer");
        }

        return resolve(mixer()->masterOutputParams());
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

void AudioOutputHandler::clearMasterOutputParams()
{
    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;

        IF_ASSERT_FAILED(mixer()) {
            return;
        }

        mixer()->clearMasterOutputParams();
    }, AudioThread::ID);
}

Channel<AudioOutputParams> AudioOutputHandler::masterOutputParamsChanged() const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;

    return m_masterOutputParamsChanged;
}

Promise<AudioResourceMetaList> AudioOutputHandler::availableOutputResources() const
{
    return Promise<AudioResourceMetaList>([this](auto resolve, auto /*reject*/) {
        ONLY_AUDIO_WORKER_THREAD;

        return resolve(fxResolver()->resolveAvailableResources());
    }, AudioThread::ID);
}

Promise<AudioSignalChanges> AudioOutputHandler::signalChanges(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    return Promise<AudioSignalChanges>([this, sequenceId, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (!s) {
            return reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }

        if (!s->audioIO()->isHasTrack(trackId)) {
            return reject(static_cast<int>(Err::InvalidTrackId), "no track");
        }

        return resolve(s->audioIO()->audioSignalChanges(trackId));
    }, AudioThread::ID);
}

Promise<AudioSignalChanges> AudioOutputHandler::masterSignalChanges() const
{
    return Promise<AudioSignalChanges>([this](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        IF_ASSERT_FAILED(mixer()) {
            return reject(static_cast<int>(Err::Undefined), "undefined reference to a mixer");
        }

        return resolve(mixer()->masterAudioSignalChanges());
    }, AudioThread::ID);
}

Promise<bool> AudioOutputHandler::saveSoundTrack(const TrackSequenceId sequenceId, const io::path_t& destination,
                                                 const SoundTrackFormat& format)
{
    return Promise<bool>([this, sequenceId, destination, format](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        IF_ASSERT_FAILED(mixer()) {
            return reject(static_cast<int>(Err::Undefined), "undefined reference to a mixer");
        }

        ITrackSequencePtr s = sequence(sequenceId);
        if (!s) {
            return reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }

#ifdef MUSE_MODULE_AUDIO_EXPORT
        s->player()->stop();
        s->player()->seek(0);
        msecs_t totalDuration = s->player()->duration();

        SoundTrackWriterPtr writer = std::make_shared<SoundTrackWriter>(destination, format, totalDuration, mixer(), iocContext());
        m_saveSoundTracksWritersMap[sequenceId] = writer;

        Progress progress = saveSoundTrackProgress(sequenceId);
        writer->progress().progressChanged.onReceive(this, [&progress](int64_t current, int64_t total, std::string title) {
            progress.progressChanged.send(current, total, title);
        });

        Ret ret = writer->write();
        s->player()->seek(0);

        m_saveSoundTracksWritersMap.erase(sequenceId);

        if (!ret) {
            return reject(ret.code(), ret.text());
        }

        return resolve(ret);
#else
        return reject(static_cast<int>(Err::DisabledAudioExport), "audio export is disabled");
#endif
    }, AudioThread::ID);
}

void AudioOutputHandler::abortSavingAllSoundTracks()
{
#ifdef MUSE_MODULE_AUDIO_EXPORT
    for (auto writer : m_saveSoundTracksWritersMap) {
        writer.second->abort();
    }
#endif
}

Progress AudioOutputHandler::saveSoundTrackProgress(const TrackSequenceId sequenceId)
{
    if (!contains(m_saveSoundTracksProgressMap, sequenceId)) {
        m_saveSoundTracksProgressMap.emplace(sequenceId, Progress());
    }

    return m_saveSoundTracksProgressMap[sequenceId];
}

void AudioOutputHandler::clearAllFx()
{
    fxResolver()->clearAllFx();
}

std::shared_ptr<Mixer> AudioOutputHandler::mixer() const
{
    return audioEngine()->mixer();
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
