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

#pragma once

#include <cstddef>
#include <map>
#include <vector>

#include "engraving/automation/automationtypes.h"
#include "engraving/types/bps.h"

namespace mu::engraving {
using PausesMap = std::map<int, double>; // utick -> held seconds
using TempoValues = std::vector<std::pair<int, double> >; // utick -> bps, sorted by utick, no duplicate uticks

struct TempoTimePoint {
    int utick = 0;
    double time = 0.0;  // cumulative nominal elapsed time at utick, pauses already included
    double bps = 0.0;   // tempo held flat from utick until the next point (or indefinitely, for the last one)
    double pause = 0.0; // seconds held at utick before "time" was reached, if this point is a pause
};

class TempoTimeline
{
public:
    void rebuild(const AutomationCurve& tempoCurve, const PausesMap& pauses);
    void rebuild(const TempoValues& tempoValues, const PausesMap& pauses);

    double utick2utime(int utick) const;
    int utime2utick(double time) const;
    BeatsPerSecond tempo(int utick) const;

    const PausesMap& pauses() const { return m_pauses; }
    const std::vector<TempoTimePoint>& points() const { return m_points; }

    const BeatsPerSecond& tempoMultiplier() const { return m_tempoMultiplier; }
    void setTempoMultiplier(const BeatsPerSecond& bps);

private:
    std::vector<TempoTimePoint> m_points; // sorted by utick for O(logN) lookup
    PausesMap m_pauses;
    BeatsPerSecond m_tempoMultiplier = 1.0;

    mutable size_t m_utick2utimeHint = 0; // cached m_points index for utick2utime()
    mutable size_t m_utime2utickHint = 0; // cached m_points index for utime2utick()
    mutable size_t m_tempoHint = 0; // cached m_points index for tempo()
};
}
