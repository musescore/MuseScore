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

inline QTime timeFromMilliseconds(audio::msecs_t millisecons)
{
    return ZERO_TIME.addMSecs(millisecons);
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
}

#endif // MU_PLAYBACK_PLAYBACKTYPES_H
