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
#ifndef MU_ENGRAVING_BPS_H
#define MU_ENGRAVING_BPS_H

#include "realfn.h"

namespace mu::engraving {
struct BeatsPerMinute // beats per minute
{
    double val = 0.0;
    BeatsPerMinute() = default;
    constexpr BeatsPerMinute(double v)
        : val(v) {}

    inline bool operator ==(const BeatsPerMinute& other) const { return muse::RealIsEqual(val, other.val); }
    inline bool operator !=(const BeatsPerMinute& other) const { return !operator ==(other); }
    inline bool operator >(const BeatsPerMinute& other) const { return val > other.val; }

    inline BeatsPerMinute operator*(const BeatsPerMinute& v) const { return BeatsPerMinute(val * v.val); }
    inline BeatsPerMinute operator/(const BeatsPerMinute& v) const { return BeatsPerMinute(val / v.val); }
    inline BeatsPerMinute operator*(const double& v) const { return BeatsPerMinute(val * v); }
    inline BeatsPerMinute operator/(const double& v) const { return BeatsPerMinute(val / v); }
};

struct BeatsPerSecond // beats per second
{
    double val = 0.0;
    BeatsPerSecond() = default;
    constexpr BeatsPerSecond(double v)
        : val(v) {}

    inline bool operator ==(const BeatsPerSecond& other) const { return muse::RealIsEqual(val, other.val); }
    inline bool operator !=(const BeatsPerSecond& other) const { return !operator ==(other); }
    inline bool operator >(const BeatsPerSecond& other) const { return val > other.val; }

    inline BeatsPerSecond operator*(const BeatsPerSecond& v) const { return BeatsPerSecond(val * v.val); }
    inline BeatsPerSecond operator/(const BeatsPerSecond& v) const { return BeatsPerSecond(val / v.val); }
    inline BeatsPerSecond operator*(const double& v) const { return BeatsPerSecond(val * v); }
    inline BeatsPerSecond operator/(const double& v) const { return BeatsPerSecond(val / v); }

    BeatsPerMinute toBPM() const { return BeatsPerMinute(val * 60.0); }
    static BeatsPerSecond fromBPM(const BeatsPerMinute& bmp) { return BeatsPerSecond(bmp.val / 60.0); }
};
}

#endif // MU_ENGRAVING_BPS_H
