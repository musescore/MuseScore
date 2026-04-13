/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "synthresolver.h"

#include "audio/common/audiosanitizer.h"

#include "log.h"

using namespace muse::async;
using namespace muse::audio;
using namespace muse::audio::synth;

void SynthResolver::init(const AudioInputParams& defaultInputParams, const OutputSpec& defaultOutputSpec)
{
    ONLY_AUDIO_ENGINE_THREAD;

    IF_ASSERT_FAILED(defaultInputParams.isValid() && defaultOutputSpec.isValid()) {
        return;
    }

    m_defaultInputParams = defaultInputParams;
    m_defaultOutputSpec = defaultOutputSpec;
}

ISynthesizerPtr SynthResolver::resolveSynth(const TrackId trackId, const AudioInputParams& params, const audio::OutputSpec& spec,
                                            const PlaybackSetupData& setupData) const
{
    ONLY_AUDIO_ENGINE_THREAD;

    TRACEFUNC;

    if (!params.isValid()) {
        LOGE() << "invalid audio source params for trackId: " << trackId;
        return nullptr;
    }

    std::lock_guard lock(m_mutex);

    auto search = m_resolvers.find(params.type());

    if (search == m_resolvers.end()) {
        return nullptr;
    }

    const IResolverPtr& resolver = search->second;

    if (!resolver->hasCompatibleResources(setupData)) {
        return nullptr;
    }

    return resolver->resolveSynth(trackId, params, spec);
}

ISynthesizerPtr SynthResolver::resolveDefaultSynth(const TrackId trackId) const
{
    ONLY_AUDIO_ENGINE_THREAD;

    return resolveSynth(trackId, m_defaultInputParams, m_defaultOutputSpec, {});
}

AudioInputParams SynthResolver::resolveDefaultInputParams() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    return m_defaultInputParams;
}

AudioResourceMetaList SynthResolver::resolveAvailableResources() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    TRACEFUNC;

    std::lock_guard lock(m_mutex);

    AudioResourceMetaList result;

    for (const auto& pair : m_resolvers) {
        const AudioResourceMetaList& resolvedResources = pair.second->resolveResources();
        result.insert(result.end(), resolvedResources.begin(), resolvedResources.end());
    }

    return result;
}

SoundPresetList SynthResolver::resolveAvailableSoundPresets(const AudioResourceMeta& resourceMeta) const
{
    ONLY_AUDIO_ENGINE_THREAD;

    TRACEFUNC;

    std::lock_guard lock(m_mutex);

    auto search = m_resolvers.find(audio::sourceTypeFromResourceType(resourceMeta.type));
    if (search == m_resolvers.end()) {
        return SoundPresetList();
    }

    return search->second->resolveSoundPresets(resourceMeta);
}

void SynthResolver::registerResolver(const AudioSourceType type, IResolverPtr resolver)
{
    ONLY_AUDIO_MAIN_OR_ENGINE_THREAD;

    std::lock_guard lock(m_mutex);

    m_resolvers.insert_or_assign(type, std::move(resolver));
}

void SynthResolver::clearSources()
{
    ONLY_AUDIO_ENGINE_THREAD;

    std::lock_guard lock(m_mutex);

    for (auto it = m_resolvers.begin(); it != m_resolvers.end(); ++it) {
        it->second->clearSources();
    }
}
