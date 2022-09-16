/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "soundprofilesrepository.h"

#include "audio/itracks.h"
#include "log.h"

using namespace mu::playback;
using namespace mu::audio;

void SoundProfilesRepository::refresh()
{
    playback()->tracks()->availableInputResources()
    .onResolve(this, [this](const AudioResourceMetaList& availableResources) {
        const SoundProfileName& basicProfileName = config()->basicSoundProfileName();
        const SoundProfileName& museProfileName = config()->museSoundProfileName();

        SoundProfile& basicProfile = m_profilesMap[basicProfileName];
        basicProfile.type = SoundProfileType::Basic;
        basicProfile.name = basicProfileName;

        SoundProfile& museProfile = m_profilesMap[museProfileName];
        museProfile.type = SoundProfileType::Muse;
        museProfile.name = museProfileName;

        for (const AudioResourceMeta& resource : availableResources) {
            auto setup = resource.attributes.find(u"playbackSetupData");

            if (setup == resource.attributes.cend()) {
                continue;
            }

            if (resource.type == AudioResourceType::FluidSoundfont) {
                basicProfile.data.emplace(mpe::PlaybackSetupData::fromString(setup->second), resource);
            }

            if (resource.type == AudioResourceType::MuseSamplerSoundPack) {
                museProfile.data.emplace(mpe::PlaybackSetupData::fromString(setup->second), resource);
            }
        }
    })
    .onReject(this, [](const int errCode, const std::string& errText) {
        LOGE() << "Unable to resolve available output resources"
               << " , errCode:" << errCode
               << " , errText:" << errText;
    });
}

const SoundProfile& SoundProfilesRepository::profile(const SoundProfileName& name) const
{
    auto search = m_profilesMap.find(name);
    if (search == m_profilesMap.cend()) {
        static SoundProfile empty;
        return empty;
    }

    return search->second;
}

const SoundProfilesMap& SoundProfilesRepository::availableProfiles() const
{
    return m_profilesMap;
}

void SoundProfilesRepository::addProfile(const SoundProfile& profile)
{
    m_profilesMap[profile.name] = profile;
}

void SoundProfilesRepository::removeProfile(const SoundProfileName& name)
{
    m_profilesMap.erase(name);
}
