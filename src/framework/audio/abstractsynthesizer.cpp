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

#include "abstractsynthesizer.h"

#include "internal/audiosanitizer.h"

using namespace mu;
using namespace mu::mpe;
using namespace mu::audio;
using namespace mu::audio::synth;

AbstractSynthesizer::AbstractSynthesizer(const AudioInputParams& params)
    : m_params(params)
{
    ONLY_AUDIO_WORKER_THREAD;
}

const AudioInputParams& AbstractSynthesizer::params() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_params;
}

async::Channel<AudioInputParams> AbstractSynthesizer::paramsChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_paramsChanges;
}

void AbstractSynthesizer::setup(const mpe::PlaybackData& playbackData)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_setupData = playbackData.setupData;

    setupSound(playbackData.setupData);
    setupEvents(playbackData);
}

void AbstractSynthesizer::revokePlayingNotes()
{
    ONLY_AUDIO_WORKER_THREAD;
}

audio::msecs_t AbstractSynthesizer::samplesToMsecs(const samples_t samplesPerChannel, const samples_t sampleRate) const
{
    ONLY_AUDIO_WORKER_THREAD;

    return samplesPerChannel * 1000000 / sampleRate;
}

samples_t AbstractSynthesizer::microSecsToSamples(const msecs_t msec, const samples_t sampleRate) const
{
    ONLY_AUDIO_WORKER_THREAD;

    return (msec / 1000.f) * sampleRate;
}
