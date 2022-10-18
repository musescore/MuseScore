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

#ifndef MU_VST_VSTIRESOLVER_H
#define MU_VST_VSTIRESOLVER_H

#include <map>

#include "modularity/ioc.h"
#include "audio/isynthresolver.h"

#include "ivstpluginsregister.h"
#include "ivstmodulesrepository.h"
#include "vstsynthesiser.h"

namespace mu::vst {
class VstiResolver : public audio::synth::ISynthResolver::IResolver
{
    INJECT(vst, IVstModulesRepository, pluginModulesRepo)
    INJECT(vst, IVstPluginsRegister, pluginsRegister)
public:
    audio::synth::ISynthesizerPtr resolveSynth(const audio::TrackId trackId, const audio::AudioInputParams& params) const override;
    bool hasCompatibleResources(const audio::PlaybackSetupData& setup) const override;
    audio::AudioResourceMetaList resolveResources() const override;
    void refresh() override;
    void clearSources() override;

private:
    VstSynthPtr createSynth(const audio::TrackId trackId, const audio::AudioInputParams& params) const;
};
}

#endif // MU_VST_VSTIRESOLVER_H
