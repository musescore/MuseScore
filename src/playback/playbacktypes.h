//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#include <cmath>

namespace mu::playback {
static const QTime ZERO_TIME(0, 0, 0, 0);

inline QTime timeFromSeconds(float seconds, int precision = 1)
{
    float secondsPart = 0;

    float frac = std::modf(seconds, &secondsPart);
    int milliseconds = static_cast<int>(frac * std::pow(10, precision));

    QTime time = ZERO_TIME;
    time = time.addSecs(static_cast<int>(secondsPart));
    time = time.addMSecs(milliseconds);

    return time;
}

inline QTime timeFromMillisecons(uint64_t millisecons)
{
    return ZERO_TIME.addMSecs(millisecons);
}

inline uint64_t timeToMilliseconds(const QTime& time)
{
    return ZERO_TIME.msecsTo(time);
}
}

#endif // MU_PLAYBACK_PLAYBACKTYPES_H
