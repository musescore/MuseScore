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

#include "abstractfxresolver.h"

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::fx;

std::vector<IFxProcessorPtr> AbstractFxResolver::resolveFxList(const TrackId trackId, const AudioFxChain& fxChain)
{
    if (fxChain.empty()) {
        LOGE() << "invalid fx chain for trackId: " << trackId;
        return {};
    }

    FxMap& fxMap = m_tracksFxMap[trackId];
    updateTrackFxMap(fxMap, trackId, fxChain);

    std::vector<IFxProcessorPtr> result;

    for (const auto& pair : fxMap) {
        result.emplace_back(pair.second);
    }

    return result;
}

std::vector<IFxProcessorPtr> AbstractFxResolver::resolveMasterFxList(const AudioFxChain& fxChain)
{
    if (fxChain.empty()) {
        LOGE() << "invalid master fx params";
        return {};
    }

    updateMasterFxMap(fxChain);

    std::vector<IFxProcessorPtr> result;

    for (const auto& pair : m_masterFxMap) {
        result.emplace_back(pair.second);
    }

    return result;
}

void AbstractFxResolver::refresh()
{
}

void AbstractFxResolver::clearAllFx()
{
    m_tracksFxMap.clear();
    m_masterFxMap.clear();
}

void AbstractFxResolver::updateTrackFxMap(FxMap& fxMap, const audio::TrackId trackId, const AudioFxChain& newFxChain)
{
    AudioFxChain currentFxChain;
    for (const auto& pair : fxMap) {
        currentFxChain.emplace(pair.first, pair.second->params());
    }

    audio::AudioFxChain fxToRemove;
    fxChainToRemove(currentFxChain, newFxChain, fxToRemove);

    for (const auto& pair : fxToRemove) {
        removeTrackFx(trackId, pair.second.resourceMeta.id, pair.second.chainOrder);

        currentFxChain.erase(pair.first);
        fxMap.erase(pair.first);
    }

    audio::AudioFxChain fxToCreate;
    fxChainToCreate(currentFxChain, newFxChain, fxToCreate);

    for (const auto& pair : fxToCreate) {
        IFxProcessorPtr fxPtr = createTrackFx(trackId, pair.second);
        if (fxPtr) {
            fxMap.emplace(pair.first, std::move(fxPtr));
        }
    }
}

void AbstractFxResolver::updateMasterFxMap(const AudioFxChain& newFxChain)
{
    AudioFxChain currentFxChain;
    for (const auto& pair : m_masterFxMap) {
        currentFxChain.emplace(pair.first, pair.second->params());
    }

    audio::AudioFxChain fxToRemove;
    fxChainToRemove(currentFxChain, newFxChain, fxToRemove);

    for (const auto& pair : fxToRemove) {
        removeMasterFx(pair.second.resourceMeta.id, pair.second.chainOrder);

        currentFxChain.erase(pair.first);
        m_masterFxMap.erase(pair.first);
    }

    audio::AudioFxChain fxToCreate;
    fxChainToCreate(currentFxChain, newFxChain, fxToCreate);

    for (const auto& pair : fxToCreate) {
        IFxProcessorPtr fx = createMasterFx(pair.second);
        if (fx) {
            m_masterFxMap.emplace(pair.first, fx);
        }
    }
}

void AbstractFxResolver::removeMasterFx(const AudioResourceId&, AudioFxChainOrder)
{
}

void AbstractFxResolver::removeTrackFx(const audio::TrackId, const AudioResourceId&, AudioFxChainOrder)
{
}

void AbstractFxResolver::fxChainToRemove(const AudioFxChain& currentFxChain,
                                         const AudioFxChain& newFxChain,
                                         AudioFxChain& resultChain)
{
    for (auto it = currentFxChain.cbegin(); it != currentFxChain.cend(); ++it) {
        auto newIt = newFxChain.find(it->first);

        if (newIt == newFxChain.cend()) {
            resultChain.insert({ it->first, it->second });
            continue;
        }

        if (it->second.resourceMeta != newIt->second.resourceMeta) {
            resultChain.insert({ it->first, it->second });
        }
    }
}

void AbstractFxResolver::fxChainToCreate(const AudioFxChain& currentFxChain,
                                         const AudioFxChain& newFxChain,
                                         AudioFxChain& resultChain)
{
    for (auto it = newFxChain.cbegin(); it != newFxChain.cend(); ++it) {
        if (currentFxChain.find(it->first) != currentFxChain.cend()) {
            continue;
        }

        if (it->second.isValid()) {
            resultChain.insert({ it->first, it->second });
        }
    }
}
