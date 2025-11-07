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

#include "note.h"

namespace mu::iex::tabledit {
// return TablEdit note length in 64th (including triplets rounded down to nearest note length)
// TODO: remove code duplication with importtef.cpp duration2length()

int durationToInt(uint8_t duration)
{
    switch (duration) {
    case  0: return 64; //"whole";
    case  1: return 48; //"half dotted";
    case  2: return 32; //"whole triplet";
    case  3: return 32; //"half";
    case  4: return 24; //"quarter dotted";
    case  5: return 16; //"half triplet";
    case  6: return 16; //"quarter";
    case  7: return 12; //"eighth dotted";
    case  8: return 8; //"quarter triplet";
    case  9: return 8; //"eighth";
    case 10: return 6; //"16th dotted";
    case 11: return 4; //"eighth triplet";
    case 12: return 4; //"16th";
    case 13: return 3; //"32nd dotted";
    case 14: return 2; //"16th triplet";
    case 15: return 2; //"32nd";
    //case 16: return "64th dotted";
    case 17: return 1; //"32nd triplet";
    case 18: return 1; //"64th";
    case 19: return 56; //"half double dotted";
    //case 20: return "16th quintuplet";
    case 22: return 28; //"quarter double dotted";
    case 25: return 14; //"eighth double dotted";
    case 28: return 7; //"16th double dotted";
    default: return 0; //"undefined";
    }
}
} // namespace mu::iex::tabledit
