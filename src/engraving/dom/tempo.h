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
inline constexpr int TEMPO_PRECISION = 6;

enum class TempoType : char {
    INVALID = 0x0, PAUSE = 0x1, FIX = 0x2, RAMP = 0x4
};

typedef muse::Flags<TempoType> TempoTypes;
DECLARE_OPERATORS_FOR_FLAGS(TempoTypes)

struct TEvent {
    TempoTypes type = TempoType::INVALID;
    BeatsPerSecond tempo = 0.0;
    double pause = 0.0; // pause in seconds
    double time = 0.0;  // precomputed time for tick in sec

    TEvent() = default;
    TEvent(BeatsPerSecond, double pauseInSeconds, TempoType);
    bool valid() const;

    bool operator ==(const TEvent& other) const
    {
        return type == other.type
               && tempo == other.tempo
               && pause == other.pause
               && time == other.time;
    }
};

class TempoMap : public std::map<int, TEvent>
{
    OBJECT_ALLOCATOR(engraving, TempoMap)

public:
    TempoMap() = default;

    void clear();
    void clearRange(int tick1, int tick2);

    void dump() const;

    BeatsPerSecond tempo(int tick) const;
    BeatsPerSecond multipliedTempo(int tick) const;
    double pauseSecs(int tick) const;

    double tick2time(int tick) const;
    int time2tick(double time) const;

    void setTempo(int t, BeatsPerSecond);
    void setPause(int t, double);
    void delTempo(int tick);

    BeatsPerSecond tempoMultiplier() const;
    bool setTempoMultiplier(BeatsPerSecond val);

private:
    void normalize();

    BeatsPerSecond m_tempo = 2.0; // tempo if not using tempo list (beats per second)
    BeatsPerSecond m_tempoMultiplier = 1.0;

    std::unordered_map<int, double> m_pauses;
};
}
