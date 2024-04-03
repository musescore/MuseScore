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

#ifndef MUSE_AUDIO_ISYNTHRESOLVER_H
#define MUSE_AUDIO_ISYNTHRESOLVER_H

#include <memory>

#include "modularity/imoduleinterface.h"

#include "isynthesizer.h"
#include "audiotypes.h"

namespace muse::audio::synth {
class ISynthResolver : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISynthResolver)

public:
    virtual ~ISynthResolver() = default;

    class IResolver
    {
    public:
        virtual ~IResolver() = default;

        virtual ISynthesizerPtr resolveSynth(const TrackId trackId, const AudioInputParams& params) const = 0;
        virtual bool hasCompatibleResources(const PlaybackSetupData& setup) const = 0;
        virtual AudioResourceMetaList resolveResources() const = 0;
        virtual SoundPresetList resolveSoundPresets(const AudioResourceMeta& resourceMeta) const = 0;
        virtual void refresh() = 0;
        virtual void clearSources() = 0;
    };
    using IResolverPtr = std::shared_ptr<IResolver>;

    virtual void init(const AudioInputParams& defaultInputParams) = 0;

    virtual ISynthesizerPtr resolveSynth(const TrackId trackId, const AudioInputParams& params,
                                         const PlaybackSetupData& setupData) const = 0;
    virtual ISynthesizerPtr resolveDefaultSynth(const TrackId trackId) const = 0;
    virtual AudioInputParams resolveDefaultInputParams() const = 0;
    virtual AudioResourceMetaList resolveAvailableResources() const = 0;
    virtual SoundPresetList resolveAvailableSoundPresets(const AudioResourceMeta& resourceMeta) const = 0;
    virtual void registerResolver(const AudioSourceType type, IResolverPtr resolver) = 0;
    virtual void clearSources() = 0;
};

using ISynthResolverPtr = std::shared_ptr<ISynthResolver>;
}

#endif // MUSE_AUDIO_ISYNTHRESOLVER_H
