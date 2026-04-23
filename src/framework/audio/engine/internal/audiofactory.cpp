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

#include "audiofactory.h"

#include "audio/common/audioerrors.h"

#include "eventaudiosource.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::engine;

AudioResourceMetaList AudioFactory::availableInputResources() const
{
    return synthResolver()->resolveAvailableResources();
}

SoundPresetList AudioFactory::availableSoundPresets(const AudioResourceMeta& resourceMeta) const
{
    return synthResolver()->resolveAvailableSoundPresets(resourceMeta);
}

AudioResourceMetaList AudioFactory::availableOutputResources() const
{
    return fxResolver()->resolveAvailableResources();
}

RetVal<synth::ISynthesizerPtr> AudioFactory::makeSynth(const TrackId trackId, const AudioInputParams& params,
                                                       const PlaybackSetupData& setupData) const
{
    auto synth = synthResolver()->resolveSynth(trackId, params, audioEngine()->outputSpec(), setupData);
    if (!synth) {
        return RetVal<synth::ISynthesizerPtr>::make_ret(Err::InvalidSynth);
    }
    return RetVal<synth::ISynthesizerPtr>::make_ok(synth);
}

RetVal<synth::ISynthesizerPtr> AudioFactory::makeDefaultSynth(const TrackId trackId) const
{
    auto synth = synthResolver()->resolveDefaultSynth(trackId);
    if (!synth) {
        return RetVal<synth::ISynthesizerPtr>::make_ret(Err::InvalidSynth);
    }
    return RetVal<synth::ISynthesizerPtr>::make_ok(synth);
}

void AudioFactory::clearSynthSources()
{
    synthResolver()->clearSources();
}

RetVal<ITrackAudioInputPtr> AudioFactory::makeEventSource(const TrackId trackId, const mpe::PlaybackData& playbackData,
                                                          const AudioInputParams& params,
                                                          const std::function<void(const TrackId)> onOffStreamReceived) const
{
    EventAudioSourcePtr source = std::make_shared<EventAudioSource>(trackId, playbackData, onOffStreamReceived);
    source->setOutputSpec(audioEngine()->outputSpec());
    source->applyInputParams(params);
    return RetVal<ITrackAudioInputPtr>::make_ok(source);
}

RetVal<ITrackAudioOutputPtr> AudioFactory::makeMixerChannel(const TrackId trackId, const AudioOutputParams& params,
                                                            const ITrackAudioInputPtr& source) const
{
    auto channel = std::make_shared<MixerChannel>(trackId, audioEngine()->outputSpec(), source, nullptr);
    channel->applyOutputParams(params);
    return RetVal<ITrackAudioOutputPtr>::make_ok(channel);
}

RetVal<ITrackAudioOutputPtr> AudioFactory::makeMixerAuxChannel(const TrackId trackId, const AudioOutputParams& params) const
{
    auto channel = std::make_shared<MixerChannel>(trackId, audioEngine()->outputSpec(), nullptr);
    channel->applyOutputParams(params);
    return RetVal<ITrackAudioOutputPtr>::make_ok(channel);
}

std::vector<IFxProcessorPtr> AudioFactory::makeMasterFxList(const AudioFxChain& fxChain) const
{
    return fxResolver()->resolveMasterFxList(fxChain, audioEngine()->outputSpec());
}

std::vector<IFxProcessorPtr> AudioFactory::makeTrackFxList(const TrackId trackId, const AudioFxChain& fxChain) const
{
    return fxResolver()->resolveFxList(trackId, fxChain, audioEngine()->outputSpec());
}

void AudioFactory::clearAllFx()
{
    fxResolver()->clearAllFx();
}
