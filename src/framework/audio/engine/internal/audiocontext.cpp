/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#include "audiocontext.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::engine;

AudioContext::AudioContext(const modularity::IoCID& ctxId)
    : m_ctxId(ctxId)
{
    UNUSED(m_ctxId); // just for information

    m_mixer = std::make_shared<Mixer>();
}

Ret AudioContext::init(const OutputSpec& outputSpec, const RenderConstraints& consts)
{
    m_mixer->init(consts.desiredAudioThreadNumber, consts.minTrackCountForMultithreading);
    m_mixer->setOutputSpec(outputSpec);

    return make_ret(Ret::Code::Ok);
}

void AudioContext::setOutputSpec(const OutputSpec& outputSpec)
{
    m_mixer->setOutputSpec(outputSpec);
}

void AudioContext::setIsIdle(bool idle)
{
    m_mixer->setIsIdle(idle);
}

void AudioContext::setIsActive(bool active)
{
    m_mixer->setIsActive(active);
}

samples_t AudioContext::process(float* buffer, samples_t samplesPerChannel)
{
    return m_mixer->process(buffer, samplesPerChannel);
}

std::shared_ptr<IAudioSource> AudioContext::output() const
{
    return m_mixer->mixedSource();
}

void AudioContext::setPlayhead(std::shared_ptr<IPlayhead> playhead)
{
    m_mixer->setPlayhead(playhead);
}

void AudioContext::setTracksToProcessWhenIdle(const std::unordered_set<TrackId>& trackIds)
{
    m_mixer->setTracksToProcessWhenIdle(trackIds);
}

AudioOutputParams AudioContext::masterOutputParams() const
{
    return m_mixer->masterOutputParams();
}

void AudioContext::setMasterOutputParams(const AudioOutputParams& params)
{
    m_mixer->setMasterOutputParams(params);
}

void AudioContext::clearMasterOutputParams()
{
    m_mixer->clearMasterOutputParams();
}

async::Channel<AudioOutputParams> AudioContext::masterOutputParamsChanged() const
{
    return m_mixer->masterOutputParamsChanged();
}

RetVal<MixerChannelPtr> AudioContext::addChannel(const TrackId trackId, ITrackAudioInputPtr source)
{
    return m_mixer->addChannel(trackId, source);
}

RetVal<MixerChannelPtr> AudioContext::addAuxChannel(const TrackId trackId)
{
    return m_mixer->addAuxChannel(trackId);
}

Ret AudioContext::removeChannel(const TrackId trackId)
{
    return m_mixer->removeChannel(trackId);
}

AudioSignalChanges AudioContext::masterAudioSignalChanges() const
{
    return m_mixer->masterAudioSignalChanges();
}
