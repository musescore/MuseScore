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

#ifndef MUSE_AUDIO_IFXRESOLVER_H
#define MUSE_AUDIO_IFXRESOLVER_H

#include <memory>

#include "modularity/imoduleinterface.h"

#include "ifxprocessor.h"
#include "audiotypes.h"

namespace muse::audio::fx {
class IFxResolver : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IFxResolver)

public:
    virtual ~IFxResolver() = default;

    class IResolver
    {
    public:
        virtual ~IResolver() = default;

        virtual std::vector<IFxProcessorPtr> resolveFxList(const audio::TrackId trackId, const AudioFxChain& fxChain) = 0;
        virtual std::vector<IFxProcessorPtr> resolveMasterFxList(const AudioFxChain& fxChain) = 0;
        virtual AudioResourceMetaList resolveResources() const = 0;
        virtual void refresh() = 0;
        virtual void clearAllFx() = 0;
    };
    using IResolverPtr = std::shared_ptr<IResolver>;

    virtual std::vector<IFxProcessorPtr> resolveMasterFxList(const AudioFxChain& fxChain) = 0;
    virtual std::vector<IFxProcessorPtr> resolveFxList(const TrackId trackId, const AudioFxChain& fxChain) = 0;
    virtual AudioResourceMetaList resolveAvailableResources() const = 0;
    virtual void registerResolver(const AudioFxType type, IResolverPtr resolver) = 0;
    virtual void clearAllFx() = 0;
};

using IFxResolverPtr = std::shared_ptr<IFxResolver>;
}

#endif // MUSE_AUDIO_IFXRESOLVER_H
