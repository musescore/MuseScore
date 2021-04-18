//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef MU_PLAYBACK_PLAYBACKTYPES_H
#define MU_PLAYBACK_PLAYBACKTYPES_H

#include <QTime>

namespace mu::playback {
enum class PlaybackCursorType {
    SMOOTH,
    STEPPED
};

static const QTime ZERO_TIME(0, 0, 0, 0);

inline uint64_t secondsToMilliseconds(float seconds)
{
    return seconds * 1000;
}

inline QTime timeFromMilliseconds(uint64_t millisecons)
{
    return ZERO_TIME.addMSecs(millisecons);
}

inline QTime timeFromSeconds(float seconds)
{
    uint64_t milliseconds = secondsToMilliseconds(seconds);
    return timeFromMilliseconds(milliseconds);
}

inline uint64_t timeToMilliseconds(const QTime& time)
{
    return ZERO_TIME.msecsTo(time);
}
}

#endif // MU_PLAYBACK_PLAYBACKTYPES_H
