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

#ifndef MU_PLAYBACK_PLAYBACKTYPES_H
#define MU_PLAYBACK_PLAYBACKTYPES_H

#include <QTime>

#include "audio/audiotypes.h"

namespace mu::playback {
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

inline audio::msecs_t secondsToMilliseconds(float seconds)
{
    return seconds * 1000;
}

inline float secondsFromMilliseconds(audio::msecs_t milliseconds)
{
    return milliseconds / 1000.f;
}

inline QTime timeFromMilliseconds(audio::msecs_t milliseconds)
{
    return ZERO_TIME.addMSecs(milliseconds);
}

inline QTime timeFromSeconds(float seconds)
{
    audio::msecs_t milliseconds = secondsToMilliseconds(seconds);
    return timeFromMilliseconds(milliseconds);
}

inline audio::msecs_t timeToMilliseconds(const QTime& time)
{
    return ZERO_TIME.msecsTo(time);
}

enum class SoundProfileType {
    Undefined = -1,
    Basic,
    Muse,
    Custom
};

using SoundProfileName = String;
using SoundProfileData = std::map<mpe::PlaybackSetupData, audio::AudioResourceMeta>;

struct SoundProfile {
    SoundProfileType type = SoundProfileType::Undefined;
    SoundProfileName name;

    SoundProfileData data;

    const audio::AudioResourceMeta& findResource(const mpe::PlaybackSetupData& key) const
    {
        auto search = data.find(key);
        if (search != data.cend()) {
            return search->second;
        }

        static audio::AudioResourceMeta empty;
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
