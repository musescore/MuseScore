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

#pragma once

#include "iaudiofactory.h"

#include "global/modularity/ioc.h"
#include "iaudioengine.h"
#include "../isynthresolver.h"
#include "../ifxresolver.h"

namespace muse::audio::engine {
class AudioFactory : public IAudioFactory
{
    GlobalInject<IAudioEngine> audioEngine;
    GlobalInject<synth::ISynthResolver> synthResolver;
    GlobalInject<fx::IFxResolver> fxResolver;

public:

    // Available resources
    AudioResourceMetaList availableInputResources() const override;
    SoundPresetList availableSoundPresets(const AudioResourceMeta& resourceMeta) const override;
    AudioResourceMetaList availableOutputResources() const override;

    // Make audio sources
    RetVal<synth::ISynthesizerPtr> makeSynth(const TrackId trackId, const AudioInputParams& params,
                                             const PlaybackSetupData& setupData) const override;
    RetVal<synth::ISynthesizerPtr> makeDefaultSynth(const TrackId trackId) const override;
    void clearSynthSources() override;

    RetVal<ITrackAudioInputPtr> makeEventSource(const TrackId trackId, const mpe::PlaybackData& playbackData,
                                                const AudioInputParams& params,
                                                const std::function<void(const TrackId)> onOffStreamReceived = nullptr) const override;

    // Make output (mixer channel)
    RetVal<ITrackAudioOutputPtr> makeMixerChannel(const TrackId trackId, const AudioOutputParams& params,
                                                  const ITrackAudioInputPtr& source) const override;
    RetVal<ITrackAudioOutputPtr> makeMixerAuxChannel(const TrackId trackId, const AudioOutputParams& params) const override;

    // Make FX
    std::vector<IFxProcessorPtr> makeMasterFxList(const AudioFxChain& fxChain) const override;
    std::vector<IFxProcessorPtr> makeTrackFxList(const TrackId trackId, const AudioFxChain& fxChain) const override;
    void clearAllFx() override;
};
}
