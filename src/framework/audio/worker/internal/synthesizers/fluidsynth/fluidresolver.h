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

#ifndef MUSE_AUDIO_FLUIDSYNTHCREATOR_H
#define MUSE_AUDIO_FLUIDSYNTHCREATOR_H

#include <optional>
#include <unordered_map>

#include "../../../isynthresolver.h"

#include "global/async/asyncable.h"
#include "global/modularity/ioc.h"

#include "isoundfontrepository.h"

#include "fluidsynth.h"

namespace muse::audio::synth {
class FluidResolver : public ISynthResolver::IResolver, public muse::Injectable, public async::Asyncable
{
    muse::Inject<ISoundFontRepository> soundFontRepository = { this };

public:
    explicit FluidResolver(const muse::modularity::ContextPtr& iocCtx);

    ISynthesizerPtr resolveSynth(const audio::TrackId trackId, const audio::AudioInputParams& params) const override;
    bool hasCompatibleResources(const audio::PlaybackSetupData& setup) const override;

    audio::AudioResourceMetaList resolveResources() const override;
    audio::SoundPresetList resolveSoundPresets(const AudioResourceMeta& resourceMeta) const override;

    void refresh() override;
    void clearSources() override;

private:
    FluidSynthPtr createSynth(const audio::AudioResourceId& resourceId) const;

    struct SoundFontResource {
        io::path_t path;
        std::optional<midi::Program> preset = std::nullopt;
        AudioResourceMeta meta;
    };

    std::unordered_map<AudioResourceId, SoundFontResource> m_resourcesCache;
};
}

#endif // MUSE_AUDIO_FLUIDSYNTHCREATOR_H
