/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
EffectType TefNote::effect() const
{
    if (simpleEffect == 0 && (complexEffect & 0x0F) == 0) {
        return EffectType::NONE;
    } else if (simpleEffect != 0 && (complexEffect & 0x0F) == 0) {
        switch (simpleEffect) {
        case   0: return EffectType::NONE;  // for completeness, but not reached
        case   1: return EffectType::HAMMER_ON;
        case   2: return EffectType::PULL_OFF;
        case   3: return EffectType::SLIDE;
        case   4: return EffectType::CHOKE;
        case   5: return EffectType::BRUSH;
        case   6: return EffectType::NATURAL_HARMONIC;
        case   7: return EffectType::ARTIFICIAL_HARMONIC;
        case   8: return EffectType::MUTED;
        case   9: return EffectType::TAPPING;
        case 0xA: return EffectType::VIBRATO;
        case 0xB: return EffectType::TREMOLO;
        case 0xC: return EffectType::SIMPLE_BEND;
        case 0xD: return EffectType::BEND_AND_RELEASE;
        case 0xE: return EffectType::ROLL_ARPEGGIO;
        case 0xF: return EffectType::DEAD_NOTE;
        default: return EffectType::INVALID;
        }
    } else if (simpleEffect == 0 && (complexEffect & 0x0F) != 0) {
        switch (complexEffect & 0x0F) {
        case 0: return EffectType::NONE;  // for completeness, but not reached
        case 1: return EffectType::RINGING_NOTE;
        case 2: return EffectType::SLAP;
        case 3: return EffectType::RASGUEADO;
        case 4: return EffectType::GHOST_NOTE;
        case 5: return EffectType::TREMOLO_UP_DOWN;
        case 6: return EffectType::TREMOLO_DIVE_RETURN;
        case 7: return EffectType::STACCATO;
        case 8: return EffectType::FADE_IN;
        case 9: return EffectType::FADE_OUT;
        default: return EffectType::INVALID;
        }
    }
    return EffectType::INVALID;
}

EffectType TefNote::combinationEffect() const
{
    switch (complexEffect & 0xF0) {
    case    0: return EffectType::NONE;
    case 0x10: return EffectType::HAMMER_ON;
    case 0x20: return EffectType::PULL_OFF;
    case 0x30: return EffectType::ROLL;
    case 0x50: return EffectType::BRUSH;
    case 0x60: return EffectType::NATURAL_HARMONIC;
    case 0x70: return EffectType::ARTIFICIAL_HARMONIC;
    case 0x80: return EffectType::RINGING_NOTE;
    case 0x90: return EffectType::GHOST_NOTE;
    case 0xB0: return EffectType::VARIATION;
    default: return EffectType::INVALID;
    }
    return EffectType::NONE;  // not reached
}

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
