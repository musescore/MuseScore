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

#ifndef __AL_TEMPO_H__
#define __AL_TEMPO_H__

#include <map>
#include <QFlags>

#include "types/types.h"

namespace Ms {
class XmlWriter;

enum class TempoType : char {
    INVALID = 0x0, PAUSE = 0x1, FIX = 0x2, RAMP = 0x4
};

typedef QFlags<TempoType> TempoTypes;
Q_DECLARE_OPERATORS_FOR_FLAGS(TempoTypes);

//---------------------------------------------------------
//   Tempo Event
//---------------------------------------------------------

struct TEvent {
    TempoTypes type;
    BeatsPerSecond tempo;       // beats per second
    qreal pause;       // pause in seconds
    qreal time;        // precomputed time for tick in sec

    TEvent();
    TEvent(const TEvent& e);
    TEvent(BeatsPerSecond bps, qreal seconds, TempoType t);
    bool valid() const;

    bool operator ==(const TEvent& other) const
    {
        return type == other.type
               && tempo == other.tempo
               && pause == other.pause
               && time == other.time;
    }
};

//---------------------------------------------------------
//   Tempomap
//---------------------------------------------------------

class TempoMap : public std::map<int, TEvent>
{
    int _tempoSN;             // serial no to track tempo changes
    BeatsPerSecond _tempo;    // tempo if not using tempo list (beats per second)
    BeatsPerSecond _relTempo;          // rel. tempo

    void normalize();
    void del(int tick);

public:
    TempoMap();
    void clear();
    void clearRange(int tick1, int tick2);

    void dump() const;

    BeatsPerSecond tempo(int tick) const;

    qreal tick2time(int tick, int* sn = 0) const;
    qreal tick2timeLC(int tick, int* sn) const;
    qreal tick2time(int tick, qreal time, int* sn) const;
    int time2tick(qreal time, int* sn = 0) const;
    int time2tick(qreal time, int tick, int* sn) const;
    int tempoSN() const { return _tempoSN; }

    void setTempo(int t, BeatsPerSecond);
    void setPause(int t, qreal);
    void delTempo(int tick);

    void setRelTempo(BeatsPerSecond val);
    BeatsPerSecond relTempo() const { return _relTempo; }
};
}     // namespace Ms
#endif
