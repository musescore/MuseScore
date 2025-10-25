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

#include <array>
#include <cstdint>

namespace mu::engraving {
struct OrnamentInterval;

struct Interval {
    int8_t diatonic = 0;
    int8_t chromatic = 0;

    Interval() = default;
    Interval(int diatonic, int chromatic);
    Interval(int chromatic);

    void flip();
    bool isZero() const;
    bool operator!=(const Interval& a) const { return diatonic != a.diatonic || chromatic != a.chromatic; }
    bool operator==(const Interval& a) const { return diatonic == a.diatonic && chromatic == a.chromatic; }

    static Interval fromOrnamentInterval(OrnamentInterval ornInt);

    /// An array of all supported interval sorted by size.
    ///
    /// Because intervals can be spelled differently, this array
    /// tracks all the different valid intervals. They are arranged
    /// in diatonic then chromatic order.
    static const std::array<Interval, 26> allIntervals;

    /// Finds the most likely diatonic interval for a semitone distance. Uses
    /// the most common diatonic intervals.
    /// @param semitones The number of semitones in the chromatic interval.
    /// Negative semitones will simply be made positive.
    /// @return The number of diatonic steps in the interval.
    static int chromatic2diatonic(int semitones);
};
}
