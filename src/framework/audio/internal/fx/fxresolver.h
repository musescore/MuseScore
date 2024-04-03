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

#ifndef MUSE_AUDIO_FXRESOLVER_H
#define MUSE_AUDIO_FXRESOLVER_H

#include <map>
#include <mutex>

#include "ifxresolver.h"

namespace muse::audio::fx {
class FxResolver : public IFxResolver
{
public:
    std::vector<IFxProcessorPtr> resolveMasterFxList(const AudioFxChain& fxChain) override;
    std::vector<IFxProcessorPtr> resolveFxList(const TrackId trackId, const AudioFxChain& fxChain) override;
    AudioResourceMetaList resolveAvailableResources() const override;
    void registerResolver(const AudioFxType type, IResolverPtr resolver) override;
    void clearAllFx() override;

private:
    std::map<AudioFxType, IResolverPtr> m_resolvers;
    mutable std::mutex m_mutex;
};
}

#endif // MUSE_AUDIO_FXRESOLVER_H
