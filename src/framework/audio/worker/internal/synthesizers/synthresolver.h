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

#ifndef MUSE_AUDIO_SYNTHRESOLVER_H
#define MUSE_AUDIO_SYNTHRESOLVER_H

#include <map>
#include <mutex>

#include "../../isynthresolver.h"

namespace muse::audio::synth {
class SynthResolver : public ISynthResolver
{
public:
    void init(const AudioInputParams& defaultInputParams) override;

    ISynthesizerPtr resolveSynth(const TrackId trackId, const AudioInputParams& params, const PlaybackSetupData& setupData) const override;
    ISynthesizerPtr resolveDefaultSynth(const TrackId trackId) const override;
    AudioInputParams resolveDefaultInputParams() const override;
    AudioResourceMetaList resolveAvailableResources() const override;
    SoundPresetList resolveAvailableSoundPresets(const AudioResourceMeta& resourceMeta) const override;

    void registerResolver(const AudioSourceType type, IResolverPtr resolver) override;

    void clearSources() override;

private:
    using SynthPair = std::pair<audio::AudioResourceId, ISynthesizerPtr>;

    mutable std::mutex m_mutex;

    std::map<AudioSourceType, IResolverPtr> m_resolvers;
    AudioInputParams m_defaultInputParams;
};
}

#endif // MUSE_AUDIO_SYNTHRESOLVER_H
