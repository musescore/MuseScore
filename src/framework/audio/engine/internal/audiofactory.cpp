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

TrackId AudioFactory::newTrackId() const
{
    static TrackId lastId = 0;
    ++lastId;
    return lastId;
}

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

RetVal<ITrackAudioOutputPtr> AudioFactory::makeMixerChannel(const TrackId trackId, const ITrackAudioInputPtr& source) const
{
    auto channel = std::make_shared<MixerChannel>(trackId, audioEngine()->outputSpec(), source, nullptr);
    return RetVal<ITrackAudioOutputPtr>::make_ok(channel);
}

RetVal<ITrackAudioOutputPtr> AudioFactory::makeMixerAuxChannel(const TrackId trackId) const
{
    auto channel = std::make_shared<MixerChannel>(trackId, audioEngine()->outputSpec(), nullptr);
    return RetVal<ITrackAudioOutputPtr>::make_ok(channel);
}

RetVal<EventTrackPtr> AudioFactory::makeEventTrack(const std::string& trackName,
                                                   const mpe::PlaybackData& playbackData,
                                                   const AudioParams& params,
                                                   std::function<void(const TrackId)> onOffStreamReceived) const
{
    if (!playbackData.setupData.isValid()) {
        return RetVal<EventTrackPtr>::make_ret(Err::InvalidSetupData);
    }

    TrackId trackId = newTrackId();

    EventAudioSourcePtr source = std::make_shared<EventAudioSource>(trackId, playbackData, onOffStreamReceived);
    source->setOutputSpec(audioEngine()->outputSpec());

    RetVal<ITrackAudioOutputPtr> channel = makeMixerChannel(trackId, source);
    if (!channel.ret) {
        return RetVal<EventTrackPtr>::make_ret(channel.ret);
    }

    EventTrackPtr trackPtr = std::make_shared<EventTrack>();
    trackPtr->id = trackId;
    trackPtr->name = trackName;
    trackPtr->setPlaybackData(playbackData);
    trackPtr->inputHandler = source;
    trackPtr->outputHandler = channel.val;
    trackPtr->setInputParams(params.in);
    trackPtr->setOutputParams(params.out);

    return RetVal<EventTrackPtr>::make_ok(trackPtr);
}

RetVal<SoundTrackPtr> AudioFactory::makeSoundTrack(const std::string& trackName,
                                                   io::IODevice* playbackData,
                                                   const AudioParams& params) const
{
    if (!playbackData) {
        return RetVal<SoundTrackPtr>::make_ret(Err::InvalidAudioFilePath);
    }

    TrackId trackId = newTrackId();
    SoundTrackPtr trackPtr = std::make_shared<SoundTrack>();
    trackPtr->id = trackId;
    trackPtr->name = trackName;
    trackPtr->setPlaybackData(playbackData);
    trackPtr->setInputParams(params.in);
    trackPtr->setOutputParams(params.out);

    return RetVal<SoundTrackPtr>::make_ok(trackPtr);
}

RetVal<EventTrackPtr> AudioFactory::makeAuxTrack(const std::string& trackName, const AudioOutputParams& outputParams) const
{
    TrackId trackId = newTrackId();
    RetVal<ITrackAudioOutputPtr> channel = makeMixerAuxChannel(trackId);
    if (!channel.ret) {
        return RetVal<EventTrackPtr>::make_ret(channel.ret);
    }

    EventTrackPtr trackPtr = std::make_shared<EventTrack>();
    trackPtr->id = trackId;
    trackPtr->name = trackName;
    trackPtr->outputHandler = channel.val;
    trackPtr->setOutputParams(outputParams);

    return RetVal<EventTrackPtr>::make_ok(trackPtr);
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
