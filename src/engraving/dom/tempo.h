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
#pragma once

#include <map>
#include <unordered_map>

#include "global/allocator.h"
#include "types/flags.h"

#include "../types/bps.h"

namespace mu::engraving {
enum class TempoType : char {
    INVALID = 0x0, PAUSE = 0x1, FIX = 0x2, RAMP = 0x4
};

typedef muse::Flags<TempoType> TempoTypes;
DECLARE_OPERATORS_FOR_FLAGS(TempoTypes)

//---------------------------------------------------------
//   Tempo Event
//---------------------------------------------------------

struct TEvent {
    TempoTypes type;
    BeatsPerSecond tempo;       // beats per second
    double pause = 0.0;       // pause in seconds
    double time = 0.0;        // precomputed time for tick in sec

    TEvent();
    TEvent(const TEvent& e);
    TEvent(BeatsPerSecond bps, double seconds, TempoType t);
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
    OBJECT_ALLOCATOR(engraving, TempoMap)

public:
    TempoMap();
    void clear();
    void clearRange(int tick1, int tick2);

    void dump() const;

    BeatsPerSecond tempo(int tick) const;
    double pauseSecs(int tick) const;

    double tick2time(int tick, int* sn = 0) const;
    double tick2time(int tick, double time, int* sn) const;
    int time2tick(double time, int* sn = 0) const;
    int time2tick(double time, int tick, int* sn) const;
    int tempoSN() const { return m_tempoSN; }

    void setTempo(int t, BeatsPerSecond);
    void setPause(int t, double);
    void delTempo(int tick);

    BeatsPerSecond tempoMultiplier() const;
    bool setTempoMultiplier(BeatsPerSecond val);

private:

    void normalize();
    void del(int tick);

    int m_tempoSN = 0; // serial no to track tempo changes
    BeatsPerSecond m_tempo; // tempo if not using tempo list (beats per second)
    BeatsPerSecond m_tempoMultiplier;

    std::unordered_map<int, double> m_pauses;
};
}
