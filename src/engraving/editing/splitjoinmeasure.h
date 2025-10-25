/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

namespace mu::engraving {
class MasterScore;
class Measure;
class Fraction;

class SplitJoinMeasure
{
public:
    /// Splits the measure at the given tick, at the given tick.
    static void splitMeasure(MasterScore* masterScore, const Fraction& tick);

    /// Joins the measures from tick1 up to (including) tick2.
    static void joinMeasures(MasterScore* masterScore, const Fraction& tick1, const Fraction& tick2);
};
}
