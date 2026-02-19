/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "modularity/ioc.h"
#include "audio/engine/isynthresolver.h"

#include "../../ivstinstancesregister.h"
#include "../../ivstmodulesrepository.h"
#include "vstsynthesiser.h"

namespace muse::vst {
class VstiResolver : public audio::synth::ISynthResolver::IResolver
{
    GlobalInject<IVstModulesRepository> pluginModulesRepo;
    GlobalInject<IVstInstancesRegister> instancesRegister;
public:
    muse::audio::synth::ISynthesizerPtr resolveSynth(const audio::TrackId trackId, const audio::AudioInputParams& params,
                                                     const audio::OutputSpec& outputSpec,
                                                     const muse::modularity::ContextPtr& iocCtx) const override;
    bool hasCompatibleResources(const muse::audio::PlaybackSetupData& setup) const override;
    muse::audio::AudioResourceMetaList resolveResources() const override;
    muse::audio::SoundPresetList resolveSoundPresets(const muse::audio::AudioResourceMeta& resourceMeta) const override;
    void refresh() override;
    void clearSources() override;

private:
    VstSynthPtr createSynth(const audio::TrackId trackId, const audio::AudioInputParams& params, const audio::OutputSpec& outputSpec,
                            const muse::modularity::ContextPtr& iocCtx) const;
};
}
