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

#include <map>
#include <unordered_map>
#include <vector>
#include <functional>

#include "modularity/ioc.h"
#include "audio/ifxresolver.h"
#include "audio/audiotypes.h"
#include "audio/ifxprocessor.h"

#include "vstfxprocessor.h"
#include "ivstpluginsregister.h"
#include "ivstmodulesrepository.h"

namespace mu::vst {
class VstFxResolver : public audio::fx::IFxResolver::IResolver
{
    INJECT(vst, IVstModulesRepository, pluginModulesRepo)
    INJECT(vst, IVstPluginsRegister, pluginsRegister)
public:
    // IFxResolver::IResolver interface
    std::vector<audio::IFxProcessorPtr> resolveFxList(const audio::TrackId trackId,
                                                      const std::vector<audio::AudioFxParams>& fxParams) override;
    std::vector<audio::IFxProcessorPtr> resolveMasterFxList(const std::vector<audio::AudioFxParams>& fxParams) override;
    audio::AudioResourceMetaList resolveResources() const override;
    void refresh() override;

private:
    using FxMap = std::unordered_map<audio::AudioResourceId, VstFxPtr>;

    VstFxPtr createMasterFx(const audio::AudioResourceId& resourceId) const;
    VstFxPtr createTrackFx(const audio::TrackId trackId, const audio::AudioResourceId& resourceId) const;

    void updateMasterFxMap(const std::vector<audio::AudioFxParams>& fxParams);
    void updateTrackFxMap(FxMap& fxMap, const audio::TrackId trackId, const std::vector<audio::AudioFxParams>& fxParams);

    void fxListToRemove(const audio::AudioResourceIdList& currentResourceIdList, const audio::AudioResourceIdList& newResourceIdList,
                        audio::AudioResourceIdList& resultList);
    void removeUnusedMasterFx(const audio::AudioResourceIdList& currentResourceIdList, const audio::AudioResourceIdList& newResourceIdList);

    void fxListToCreate(const audio::AudioResourceIdList& currentResourceIdList, const audio::AudioResourceIdList& newResourceIdList,
                        audio::AudioResourceIdList& resultList);
    void createMissingMasterFx(const audio::AudioResourceIdList& currentResourceIdList,
                               const audio::AudioResourceIdList& newResourceIdList);

    std::map<audio::TrackId, FxMap> m_tracksFxMap;
    FxMap m_masterFxMap;
};
}

#endif // MU_VST_VSTFXRESOLVER_H
