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

#ifndef MU_VST_VSTFXRESOLVER_H
#define MU_VST_VSTFXRESOLVER_H

#include "audio/abstractfxresolver.h"

#include "modularity/ioc.h"
#include "ivstpluginsregister.h"
#include "ivstmodulesrepository.h"

namespace mu::vst {
class VstFxResolver : public audio::fx::AbstractFxResolver
{
    INJECT(IVstModulesRepository, pluginModulesRepo)
    INJECT(IVstPluginsRegister, pluginsRegister)
public:
    // IFxResolver::IResolver interface
    audio::AudioResourceMetaList resolveResources() const override;
    void refresh() override;
    void clearAllFx() override;

private:
    audio::IFxProcessorPtr createMasterFx(const audio::AudioFxParams& fxParams) const override;
    audio::IFxProcessorPtr createTrackFx(const audio::TrackId trackId, const audio::AudioFxParams& fxParams) const override;

    void removeMasterFx(const audio::AudioResourceId& resoureId, audio::AudioFxChainOrder order) override;
    void removeTrackFx(const audio::TrackId trackId, const audio::AudioResourceId& resoureId, audio::AudioFxChainOrder order) override;
};
}

#endif // MU_VST_VSTFXRESOLVER_H
