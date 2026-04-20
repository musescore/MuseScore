/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include <algorithm>
#include <vector>

#include "../../dom/repeatlist.h"
#include "../../dom/score.h"

namespace mu::engraving {
inline int repeatPassForUtick(const Score* score, const int utick)
{
    if (!score) {
        return 1;
    }

    const RepeatList& repeats = score->repeatList();
    auto repeatIt = repeats.findRepeatSegmentFromUTick(utick);
    if (repeatIt == repeats.cend()) {
        return 1;
    }

    const int pass = (*repeatIt)->playbackCount;
    return pass > 0 ? pass : 1;
}

inline bool shouldApplyOnPass(const std::vector<int>& passes, const int pass)
{
    if (passes.empty()) {
        return true;
    }

    if (pass <= 0) {
        return false;
    }

    return std::find(passes.cbegin(), passes.cend(), pass) != passes.cend();
}
}
