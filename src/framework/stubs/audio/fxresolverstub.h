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
#ifndef MU_AUDIO_FXRESOLVERSTUB_H
#define MU_AUDIO_FXRESOLVERSTUB_H

#include "audio/worker/ifxresolver.h"

namespace muse::audio::fx {
class FxResolverStub : public IFxResolver
{
public:
    std::vector<IFxProcessorPtr> resolveMasterFxList(const AudioFxChain& fxChain) override;
    std::vector<IFxProcessorPtr> resolveFxList(const TrackId trackId, const AudioFxChain& fxChain) override;
    AudioResourceMetaList resolveAvailableResources() const override;
    void registerResolver(const AudioFxType type, IResolverPtr resolver) override;
    void clearAllFx() override;
};
}

#endif // MU_AUDIO_FXRESOLVERSTUB_H
