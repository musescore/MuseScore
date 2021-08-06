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

#include "fxresolver.h"

#include "log.h"

#include "audioerrors.h"
#include "internal/audiothread.h"
#include "internal/audiosanitizer.h"

using namespace mu::async;
using namespace mu::audio;
using namespace mu::audio::fx;

std::vector<IFxProcessorPtr> FxResolver::resolveMasterFxList(const AudioFxParamsMap& fxParams)
{
    ONLY_AUDIO_WORKER_THREAD;

    TRACEFUNC;

    std::lock_guard lock(m_mutex);

    std::vector<IFxProcessorPtr> result;

    for (const auto& pair : fxParams) {
        auto search = m_resolvers.find(pair.first);

        if (search == m_resolvers.end()) {
            continue;
        }

        std::vector<IFxProcessorPtr> fxList = search->second->resolveMasterFxList(pair.second);
        result.insert(result.end(), fxList.begin(), fxList.end());
    }

    return result;
}

std::vector<IFxProcessorPtr> FxResolver::resolveFxList(const TrackId trackId, const AudioFxParamsMap& fxParams)
{
    ONLY_AUDIO_WORKER_THREAD;

    TRACEFUNC;

    std::lock_guard lock(m_mutex);

    std::vector<IFxProcessorPtr> result;

    for (const auto& pair : fxParams) {
        auto search = m_resolvers.find(pair.first);

        if (search == m_resolvers.end()) {
            continue;
        }

        std::vector<IFxProcessorPtr> fxList = search->second->resolveFxList(trackId, pair.second);
        result.insert(result.end(), fxList.begin(), fxList.end());
    }

    return result;
}

AudioResourceIdList FxResolver::resolveAvailableResources(const AudioFxType type) const
{
    ONLY_AUDIO_WORKER_THREAD;

    TRACEFUNC;

    std::lock_guard lock(m_mutex);

    auto search = m_resolvers.find(type);

    if (search == m_resolvers.end()) {
        return {};
    }

    return search->second->resolveResources();
}

void FxResolver::registerResolver(const AudioFxType type, IResolverPtr resolver)
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;

    std::lock_guard lock(m_mutex);

    m_resolvers.insert_or_assign(type, std::move(resolver));
}
