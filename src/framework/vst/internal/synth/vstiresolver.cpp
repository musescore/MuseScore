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

#include "vstiresolver.h"

#include "log.h"

using namespace mu::vst;
using namespace mu::audio;
using namespace mu::audio::synth;

ISynthesizerPtr VstiResolver::resolveSynth(const audio::TrackId trackId, const audio::AudioInputParams& params) const
{
    return createSynth(trackId, params);
}

bool VstiResolver::hasCompatibleResources(const audio::PlaybackSetupData& /*setup*/) const
{
    return true;
}

void VstiResolver::refresh()
{
    pluginModulesRepo()->refresh();
}

VstSynthPtr VstiResolver::createSynth(const audio::TrackId trackId, const audio::AudioInputParams& params) const
{
    if (!pluginModulesRepo()->exists(params.resourceMeta.id)) {
        return nullptr;
    }

    VstPluginPtr pluginPtr = std::make_shared<VstPlugin>(params.resourceMeta.id);
    pluginsRegister()->registerInstrPlugin(trackId, pluginPtr);

    pluginPtr->load();

    return std::make_shared<VstSynthesiser>(std::move(pluginPtr), params);
}

AudioResourceMetaList VstiResolver::resolveResources() const
{
    return pluginModulesRepo()->instrumentModulesMeta();
}
