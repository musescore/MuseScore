/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "engravingpluginmixerapiv1.h"

#include <algorithm>

#include "project/iprojectaudiosettings.h"
#include "project/inotationproject.h"

using namespace mu::playback;
using namespace muse;
using namespace muse::audio;

static QString soundResourceId(const AudioResourceMeta &meta)
{
    return QString::fromStdString(meta.type + "/" + meta.id);
}

float EngravingPluginMixerApi::volume(const engraving::InstrumentTrackId &trackId) const
{
    const auto currentProject = context()->currentProject();
    if (!currentProject)
    {
        return 0.f;
    }

    return currentProject->audioSettings()->trackOutputParams(trackId).volume.raw();
}

void EngravingPluginMixerApi::setVolume(const engraving::InstrumentTrackId &trackId, float volume)
{
    const auto currentProject = context()->currentProject();
    const auto track = audioTrackId(trackId);
    if (!currentProject || !track)
    {
        return;
    }

    project::AudioOutputParams params = currentProject->audioSettings()->trackOutputParams(trackId);
    params.volume = std::clamp(volume, VOLUME_DB_MIN.raw(), VOLUME_DB_MAX.raw());
    currentProject->audioSettings()->setTrackOutputParams(trackId, params);
    playback()->setControlParams(*track, params.control());
}

float EngravingPluginMixerApi::balance(const engraving::InstrumentTrackId &trackId) const
{
    const auto currentProject = context()->currentProject();
    if (!currentProject)
    {
        return 0.f;
    }

    return currentProject->audioSettings()->trackOutputParams(trackId).balance.raw();
}

void EngravingPluginMixerApi::setBalance(const engraving::InstrumentTrackId &trackId, float balance)
{
    const auto currentProject = context()->currentProject();
    const auto track = audioTrackId(trackId);
    if (!currentProject || !track)
    {
        return;
    }

    project::AudioOutputParams params = currentProject->audioSettings()->trackOutputParams(trackId);
    params.balance = std::clamp(balance, BALANCE_MIN.raw(), BALANCE_MAX.raw());
    currentProject->audioSettings()->setTrackOutputParams(trackId, params);
    playback()->setControlParams(*track, params.control());
}

bool EngravingPluginMixerApi::muted(const engraving::InstrumentTrackId &trackId) const
{
    return controller()->trackSoloMuteState(trackId).mute;
}

void EngravingPluginMixerApi::setMuted(const engraving::InstrumentTrackId &trackId, bool muted)
{
    IPlaybackController::SoloMuteState state = controller()->trackSoloMuteState(trackId);
    state.mute = muted;
    if (muted)
    {
        state.solo = false;
    }
    controller()->setTrackSoloMuteState(trackId, state);
}

bool EngravingPluginMixerApi::solo(const engraving::InstrumentTrackId &trackId) const
{
    return controller()->trackSoloMuteState(trackId).solo;
}

void EngravingPluginMixerApi::setSolo(const engraving::InstrumentTrackId &trackId, bool solo)
{
    IPlaybackController::SoloMuteState state = controller()->trackSoloMuteState(trackId);
    state.solo = solo;
    if (solo)
    {
        state.mute = false;
    }
    controller()->setTrackSoloMuteState(trackId, state);
}

Promise<QVariantList> EngravingPluginMixerApi::availableSounds(const engraving::InstrumentTrackId &trackId) const
{
    if (!audioTrackId(trackId))
    {
        return Promise<QVariantList>([](auto, auto reject)
                                     { reject(static_cast<int>(Ret::Code::UnknownError), "invalid instrumentTrackId"); });
    }

    return Promise<QVariantList>([this](auto resolve, auto reject)
                                 { playback()->availableInputResources().onResolve(this, [resolve](const AudioResourceMetaList &resources)
                                                                                   {
            QVariantList result;
            result.reserve(static_cast<qsizetype>(resources.size()));
            for (const AudioResourceMeta& resource : resources) {
                const QString id = soundResourceId(resource);
                AudioInputParams params;
                params.resourceMeta = resource;
                const QString name = audioSourceName(params).toQString();
                result.append(QVariantMap { { "id", id }, { "name", name } });
            }
            resolve(result); })
                                       .onReject(this, [reject](int code, const std::string &error)
                                                 { reject(code, error); }); });
}

Promise<bool> EngravingPluginMixerApi::setSound(const engraving::InstrumentTrackId &trackId, const QString &soundId)
{
    const auto track = audioTrackId(trackId);
    const auto currentProject = context()->currentProject();
    if (!track || !currentProject)
    {
        return Promise<bool>([](auto resolve, auto)
                             { resolve(false); });
    }

    return Promise<bool>([this, trackId, track, soundId, currentProject](auto resolve, auto reject)
                         { playback()->availableInputResources().onResolve(this, [this, trackId, track, soundId, currentProject, resolve](const AudioResourceMetaList &resources)
                                                                           {
            const auto resource = std::find_if(resources.cbegin(), resources.cend(), [&soundId](const AudioResourceMeta& meta) {
                return soundResourceId(meta) == soundId;
            });
            if (resource == resources.cend()) {
                resolve(false);
                return;
            }

            project::AudioInputParams params = currentProject->audioSettings()->trackInputParams(trackId);
            params.resourceMeta = *resource;
            params.configuration.clear();
            playback()->setSourceParams(*track, params);
            resolve(true); })
                               .onReject(this, [reject](int code, const std::string &error)
                                         { reject(code, error); }); });
}

std::optional<TrackId> EngravingPluginMixerApi::audioTrackId(const engraving::InstrumentTrackId &trackId) const
{
    const auto &tracks = controller()->instrumentTrackIdMap();
    const auto found = tracks.find(trackId);
    if (found == tracks.end())
    {
        return std::nullopt;
    }

    return found->second;
}