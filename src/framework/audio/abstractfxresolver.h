/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#ifndef MUSE_AUDIO_ABSTRACTFXRESOLVER_H
#define MUSE_AUDIO_ABSTRACTFXRESOLVER_H

#include <map>
#include <unordered_map>

#include "ifxresolver.h"
#include "ifxprocessor.h"

namespace muse::audio::fx {
class AbstractFxResolver : public IFxResolver::IResolver
{
public:
    std::vector<IFxProcessorPtr> resolveFxList(const TrackId trackId, const AudioFxChain& fxChain) override;
    std::vector<IFxProcessorPtr> resolveMasterFxList(const AudioFxChain& fxChain) override;
    void refresh() override;
    void clearAllFx() override;

protected:
    virtual IFxProcessorPtr createMasterFx(const AudioFxParams& fxParams) const = 0;
    virtual IFxProcessorPtr createTrackFx(const TrackId trackId, const AudioFxParams& fxParams) const = 0;

    virtual void removeMasterFx(const AudioResourceId& resoureId, AudioFxChainOrder order);
    virtual void removeTrackFx(const TrackId trackId, const AudioResourceId& resoureId, AudioFxChainOrder order);

private:
    using FxMap = std::unordered_map<AudioFxChainOrder, IFxProcessorPtr>;

    void updateMasterFxMap(const AudioFxChain& newFxChain);
    void updateTrackFxMap(FxMap& fxMap, const TrackId trackId, const AudioFxChain& newFxChain);

    void fxChainToRemove(const AudioFxChain& currentFxChain, const AudioFxChain& newFxChain, AudioFxChain& resultChain);
    void fxChainToCreate(const AudioFxChain& currentFxChain, const AudioFxChain& newFxChain, AudioFxChain& resultChain);

    std::map<TrackId, FxMap> m_tracksFxMap;
    FxMap m_masterFxMap;
};
}

#endif // MUSE_AUDIO_ABSTRACTFXRESOLVER_H
