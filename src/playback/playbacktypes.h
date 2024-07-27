/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_PLAYBACK_PLAYBACKTYPES_H
#define MU_PLAYBACK_PLAYBACKTYPES_H

#include <QTime>

#include "audio/audiotypes.h"

namespace mu::playback {
static constexpr muse::audio::aux_channel_idx_t AUX_CHANNEL_NUM = 2;
static constexpr muse::audio::aux_channel_idx_t REVERB_CHANNEL_IDX = 0;

enum class PlaybackCursorType {
    SMOOTH,
    STEPPED
};

enum class MixerSectionType {
    Unknown,
    Labels,
    Sound,
    AudioFX,
    Balance,
    Volume,
    Fader,
    MuteAndSolo,
    Title
};

inline QList<MixerSectionType> allMixerSectionTypes()
{
    static const QList<MixerSectionType> sections {
        MixerSectionType::Labels,
        MixerSectionType::Sound,
        MixerSectionType::AudioFX,
        MixerSectionType::Balance,
        MixerSectionType::Volume,
        MixerSectionType::Fader,
        MixerSectionType::MuteAndSolo,
        MixerSectionType::Title
    };

    return sections;
}

static const QTime ZERO_TIME(0, 0, 0, 0);

inline QTime timeFromSeconds(muse::audio::secs_t seconds)
{
    return ZERO_TIME.addMSecs(muse::audio::secsToMilisecs(seconds));
}

inline muse::audio::secs_t timeToSeconds(const QTime& time)
{
    return muse::audio::milisecsToSecs(ZERO_TIME.msecsTo(time));
}

enum class SoundProfileType {
    Undefined = -1,
    Basic,
    Muse,
    Custom
};

using SoundProfileName = muse::String;
using SoundProfileData = std::map<muse::mpe::PlaybackSetupData, muse::audio::AudioResourceMeta>;

struct SoundProfile {
    SoundProfileType type = SoundProfileType::Undefined;
    SoundProfileName name;

    SoundProfileData data;

    const muse::audio::AudioResourceMeta& findResource(const muse::mpe::PlaybackSetupData& key) const
    {
        auto search = data.find(key);
        if (search != data.cend()) {
            return search->second;
        }

        static muse::audio::AudioResourceMeta empty;
        return empty;
    }

    bool isEnabled() const
    {
        return !data.empty();
    }

    bool isValid() const
    {
        return type != SoundProfileType::Undefined
               && !name.isEmpty()
               && isEnabled();
    }
};

using SoundProfilesMap = std::map<SoundProfileName, SoundProfile>;
}

#endif // MU_PLAYBACK_PLAYBACKTYPES_H
