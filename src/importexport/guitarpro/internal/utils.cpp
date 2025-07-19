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
#include "utils.h"

#include "realfn.h"
#include "engraving/dom/note.h"

using namespace mu::engraving;

namespace mu::iex::guitarpro::utils {
int harmonicOvertone(Note* note, float harmonicValue, int harmonicType)
{
    int result{ 0 };

    if (muse::RealIsEqual(harmonicValue, 12.0f)) {
        result = 12;
    } else if (muse::RealIsEqual(harmonicValue, 7.0f) || muse::RealIsEqual(harmonicValue, 19.0f)) {
        result = 19;
    } else if (muse::RealIsEqual(harmonicValue, 5.0f) || muse::RealIsEqual(harmonicValue, 24.0f)) {
        result = 24;
    } else if (muse::RealIsEqual(harmonicValue, 3.9f)
               || muse::RealIsEqual(harmonicValue, 4.0f)
               || muse::RealIsEqual(harmonicValue, 9.0f)
               || muse::RealIsEqual(harmonicValue, 16.0f)) {
        result = 28;
    } else if (muse::RealIsEqual(harmonicValue, 3.2f)) {
        result = 31;
    } else if (muse::RealIsEqual(harmonicValue, 2.7f)
               || muse::RealIsEqual(harmonicValue, 5.8f)
               || muse::RealIsEqual(harmonicValue, 9.6f)
               || muse::RealIsEqual(harmonicValue, 14.7f)
               || muse::RealIsEqual(harmonicValue, 21.7f)) {
        result = 34;
    } else if (muse::RealIsEqual(harmonicValue, 2.3f)
               || muse::RealIsEqual(harmonicValue, 2.4f)
               || muse::RealIsEqual(harmonicValue, 8.2f)
               || muse::RealIsEqual(harmonicValue, 17.0f)) {
        result = 36;
    } else if (muse::RealIsEqual(harmonicValue, 2.0f)) {
        result = 38;
    } else if (muse::RealIsEqual(harmonicValue, 1.8f)) {
        result = 40;
    }

    return harmonicType == 1 ? result : (result + note->fret());
}
} // namespace mu::iex::guitarpro
