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

#include <algorithm>

#include "containers.h"

#include "pitch.h"

namespace mu::engraving {
//---------------------------------------------------------
//   PitchList
//    return pitch offset at tick position (ottava)
//---------------------------------------------------------

int PitchList::pitchOffset(int tick) const
{
    if (empty()) {
        return 0;
    }

    std::vector<int> ticks = muse::keys(*this);
    auto it = std::upper_bound(ticks.cbegin(), ticks.cend(), tick);
    if (it == ticks.cbegin()) {
        return 0;
    }
    --it;
    return at(*it);
}
}
