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

#ifndef MU_ENGRAVING_SWING_H
#define MU_ENGRAVING_SWING_H

namespace mu::engraving {
class Chord;
struct SwingParameters;

class Swing
{
public:
    struct ChordDurationAdjustment
    {
        double remainingDurationMultiplier = 0.;
        double durationMultiplier = 0.;

        bool isNull() const;
    };

    static ChordDurationAdjustment applySwing(const Chord* chord, const SwingParameters& params);
    static void swingAdjustParams(const Chord* chord, const SwingParameters& params, int& onTime, int& gateTime);
};
}

#endif // MU_ENGRAVING_SWING_H
