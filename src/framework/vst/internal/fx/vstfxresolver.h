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

#ifndef MUSE_VST_VSTFXRESOLVER_H
#define MUSE_VST_VSTFXRESOLVER_H

#include "audio/worker/internal/fx/abstractfxresolver.h"

#include "modularity/ioc.h"
#include "../../ivstinstancesregister.h"
#include "../../ivstmodulesrepository.h"

namespace muse::vst {
class VstFxResolver : public muse::audio::fx::AbstractFxResolver
{
    muse::Inject<IVstModulesRepository> pluginModulesRepo;
    muse::Inject<IVstInstancesRegister> instancesRegister;
public:
    // IFxResolver::IResolver interface
    muse::audio::AudioResourceMetaList resolveResources() const override;
    void refresh() override;
    void clearAllFx() override;

private:
    muse::audio::IFxProcessorPtr createMasterFx(const muse::audio::AudioFxParams& fxParams) const override;
    muse::audio::IFxProcessorPtr createTrackFx(const muse::audio::TrackId trackId,
                                               const muse::audio::AudioFxParams& fxParams) const override;

    void removeMasterFx(const muse::audio::AudioResourceId& resoureId, muse::audio::AudioFxChainOrder order) override;
    void removeTrackFx(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resoureId,
                       muse::audio::AudioFxChainOrder order) override;
};
}

#endif // MUSE_VST_VSTFXRESOLVER_H
