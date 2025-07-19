/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "eventaudiosource.h"

#include "internal/audiosanitizer.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::synth;
using namespace muse::mpe;

EventAudioSource::EventAudioSource(const TrackId trackId,
                                   const mpe::PlaybackData& playbackData,
                                   OnOffStreamEventsReceived onOffStreamReceived,
                                   const modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx), m_trackId(trackId), m_playbackData(playbackData)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_playbackData.offStream.onReceive(this, [onOffStreamReceived, trackId](const PlaybackEventsMap&,
                                                                            const DynamicLevelLayers&,
                                                                            const PlaybackParamList&) {
        onOffStreamReceived(trackId);
    });
}

EventAudioSource::~EventAudioSource()
{
    m_playbackData.offStream.resetOnReceive(this);
}

bool EventAudioSource::isActive() const
{
    ONLY_AUDIO_WORKER_THREAD;

    if (!m_synth) {
        return false;
    }

    return m_synth->isActive();
}

void EventAudioSource::setIsActive(const bool active)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_synth) {
        return;
    }

    if (m_synth->isActive() == active) {
        return;
    }

    m_synth->setIsActive(active);
    m_synth->flushSound();
}

void EventAudioSource::setSampleRate(unsigned int sampleRate)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_sampleRate = sampleRate;

    if (!m_synth) {
        return;
    }

    m_synth->setSampleRate(sampleRate);
}

unsigned int EventAudioSource::audioChannelsCount() const
{
    ONLY_AUDIO_WORKER_THREAD;

    if (!m_synth) {
        return 0;
    }

    return m_synth->audioChannelsCount();
}

async::Channel<unsigned int> EventAudioSource::audioChannelsCountChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_synth) {
        return {};
    }

    return m_synth->audioChannelsCountChanged();
}

samples_t EventAudioSource::process(float* buffer, samples_t samplesPerChannel)
{
    ONLY_AUDIO_WORKER_THREAD;

    if (!m_synth) {
        return 0;
    }

    return m_synth->process(buffer, samplesPerChannel);
}

void EventAudioSource::seek(const msecs_t newPositionMsecs)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_synth) {
        return;
    }

    if (m_synth->playbackPosition() == newPositionMsecs) {
        return;
    }

    m_synth->setPlaybackPosition(newPositionMsecs);
    m_synth->revokePlayingNotes();
}

void EventAudioSource::flush()
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_synth) {
        return;
    }

    m_synth->flushSound();
}

const AudioInputParams& EventAudioSource::inputParams() const
{
    return m_params;
}

void EventAudioSource::applyInputParams(const AudioInputParams& requiredParams)
{
    if (m_params.isValid() && m_params == requiredParams) {
        return;
    }

    SynthCtx ctx = currentSynthCtx();

    if (m_synth) {
        m_playbackData = m_synth->playbackData();
    }

    m_synth = synthResolver()->resolveSynth(m_trackId, requiredParams, m_playbackData.setupData);

    if (!m_synth) {
        m_synth = synthResolver()->resolveDefaultSynth(m_trackId);
        IF_ASSERT_FAILED(m_synth) {
            LOGE() << "Default synth not found!";
            return;
        }
    }

    m_synth->paramsChanged().onReceive(this, [this](const AudioInputParams& params) {
        m_paramsChanges.send(params);
    });

    setupSource();

    if (ctx.isValid()) {
        restoreSynthCtx(ctx);
    } else {
        m_synth->setIsActive(false);
    }

    m_params = m_synth->params();
    m_paramsChanges.send(m_params);
}

async::Channel<AudioInputParams> EventAudioSource::inputParamsChanged() const
{
    return m_paramsChanges;
}

void EventAudioSource::prepareToPlay()
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_synth) {
        return;
    }

    m_synth->prepareToPlay();
}

bool EventAudioSource::readyToPlay() const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_synth) {
        return false;
    }

    return m_synth->readyToPlay();
}

async::Notification EventAudioSource::readyToPlayChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_synth) {
        return {};
    }

    return m_synth->readyToPlayChanged();
}

InputProcessingProgress EventAudioSource::inputProcessingProgress() const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_synth) {
        return {};
    }

    return m_synth->inputProcessingProgress();
}

EventAudioSource::SynthCtx EventAudioSource::currentSynthCtx() const
{
    if (!m_synth) {
        return SynthCtx();
    }

    return { m_synth->isActive(), m_synth->playbackPosition() };
}

void EventAudioSource::restoreSynthCtx(const SynthCtx& ctx)
{
    m_synth->setPlaybackPosition(ctx.playbackPosition);
    m_synth->setIsActive(ctx.isActive);
}

void EventAudioSource::setupSource()
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_synth) {
        return;
    }

    m_synth->setSampleRate(m_sampleRate);
    m_synth->setup(m_playbackData);
}
