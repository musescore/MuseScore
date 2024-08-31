/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#ifndef MU_ENGRAVING_HARMONYLAYOUT_DEV_H
#define MU_ENGRAVING_HARMONYLAYOUT_DEV_H

#include <vector>

#include "layoutcontext.h"

namespace mu::engraving {
class Segment;
class System;
}

namespace mu::engraving::rendering::score {
class HarmonyLayout
{
public:

    static void layoutHarmonies(const std::vector<Segment*>& sl, LayoutContext& ctx);
    static void alignHarmonies(const System* system, const std::vector<Segment*>& sl, bool harmony, const double maxShiftAbove,
                               const double maxShiftBelow);
};
}

#endif // MU_ENGRAVING_HARMONYLAYOUT_DEV_H
