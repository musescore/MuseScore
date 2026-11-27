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
#include "log.h"

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
}

//---------------------------------------------------------
// type
// convert TablEdit duration to note type in 64th
// i.e. the nominal duration (excluding dots or tuplet)
//---------------------------------------------------------

int TefNote::type() const
{
    switch (duration) {
    case NoteDuration::V_WHOLE:
    case NoteDuration::V_WHOLE_DOTTED:
    case NoteDuration::V_WHOLE_TRIPLET:
        return 64;
    case NoteDuration::V_HALF:
    case NoteDuration::V_HALF_DOTTED:
    case NoteDuration::V_HALF_DOUBLE_DOTTED:
    case NoteDuration::V_HALF_TRIPLET:
        return 32;
    case NoteDuration::V_QUARTER:
    case NoteDuration::V_QUARTER_DOTTED:
    case NoteDuration::V_QUARTER_DOUBLE_DOTTED:
    case NoteDuration::V_QUARTER_TRIPLET:
        return 16;
    case NoteDuration::V_EIGHTH:
    case NoteDuration::V_EIGHTH_DOTTED:
    case NoteDuration::V_EIGHTH_DOUBLE_DOTTED:
    case NoteDuration::V_EIGHTH_TRIPLET:
        return 8;
    case NoteDuration::V_16TH:
    case NoteDuration::V_16TH_DOTTED:
    case NoteDuration::V_16TH_DOUBLE_DOTTED:
    case NoteDuration::V_16TH_TRIPLET:
        return 4;
    case NoteDuration::V_32ND:
    case NoteDuration::V_32ND_DOTTED:
    case NoteDuration::V_32ND_TRIPLET:
        return 2;
    case NoteDuration::V_64TH:
        return 1;
    case NoteDuration::V_16TH_QUINTUPLET: // not supported by importer yet
    case NoteDuration::V_32ND_SEPTUPLET: // not supported by importer yet
    case NoteDuration::V_32ND_NONUPLET: // not supported by importer yet
    case NoteDuration::V_64TH_DOTTED:
    default:
        LOGD("invalid note duration %d", static_cast<int>(duration));
        return 0; // invalid (result is weird layout)
    }
}

//---------------------------------------------------------
// dots
// convert TablEdit duration to number of dots
//---------------------------------------------------------

int TefNote::dots() const
{
    switch (duration) {
    case NoteDuration::V_WHOLE_DOTTED:
    case NoteDuration::V_HALF_DOTTED:
    case NoteDuration::V_QUARTER_DOTTED:
    case NoteDuration::V_EIGHTH_DOTTED:
    case NoteDuration::V_16TH_DOTTED:
    case NoteDuration::V_32ND_DOTTED:
    case NoteDuration::V_64TH_DOTTED:
        return 1;
    case NoteDuration::V_HALF_DOUBLE_DOTTED:
    case NoteDuration::V_QUARTER_DOUBLE_DOTTED:
    case NoteDuration::V_EIGHTH_DOUBLE_DOTTED:
    case NoteDuration::V_16TH_DOUBLE_DOTTED:
        return 2;
    case NoteDuration::V_16TH_QUINTUPLET: // not supported by importer yet
    case NoteDuration::V_32ND_SEPTUPLET: // not supported by importer yet
    case NoteDuration::V_32ND_NONUPLET: // not supported by importer yet
    default:
        return 0;
    }
}

//---------------------------------------------------------
// tuplet
// convert TablEdit duration to tuplet type
// currently a bool (triplet yes/no), in future the tuplet value
//---------------------------------------------------------

bool TefNote::tuplet() const
{
    switch (duration) {
    case NoteDuration::V_WHOLE_TRIPLET:
    case NoteDuration::V_HALF_TRIPLET:
    case NoteDuration::V_QUARTER_TRIPLET:
    case NoteDuration::V_EIGHTH_TRIPLET:
    case NoteDuration::V_16TH_TRIPLET:
    case NoteDuration::V_32ND_TRIPLET:
        return true;
    case NoteDuration::V_16TH_QUINTUPLET: // not supported by importer yet
    case NoteDuration::V_32ND_SEPTUPLET: // not supported by importer yet
    case NoteDuration::V_32ND_NONUPLET: // not supported by importer yet
    case NoteDuration::V_64TH_DOTTED:
    default:
        return false;
    }
}

//---------------------------------------------------------
// lengthInclDots
// convert TablEdit duration to length in 64th
// i.e. the nominal duration (including dot(s), excluding tuplet)
//---------------------------------------------------------

int TefNote::lengthInclDots() const
{
    const int length { type() };
    switch (dots()) {
    case 0: return length;
    case 1: return 3 * length / 2;
    case 2: return 7 * length / 4;
    default:
        LOGD("number of dots (%d) not supported", dots());
        return 0;
    }
}

//---------------------------------------------------------
// durationToTablEditLength
// return TablEdit note length in 64th (including dots and
// triplets).
// Note TablEdit rounds down notes in tuplets to the nearest
// note length. E.g. in a eighth note triplet, the notes
// will be at 1/16th intervals with a gap after the last note
//---------------------------------------------------------

int TefNote::tablEditLength() const
{
    if (!tuplet()) {
        return lengthInclDots();
    }
    switch (duration) {
    case NoteDuration::V_WHOLE_TRIPLET: return 32;
    case NoteDuration::V_HALF_TRIPLET: return 16;
    case NoteDuration::V_QUARTER_TRIPLET: return 8;
    case NoteDuration::V_EIGHTH_TRIPLET: return 4;
    case NoteDuration::V_16TH_TRIPLET: return 2;
    case NoteDuration::V_32ND_TRIPLET: return 1;
    case NoteDuration::V_16TH_QUINTUPLET: // not supported by importer yet
    case NoteDuration::V_32ND_SEPTUPLET: // not supported by importer yet
    case NoteDuration::V_32ND_NONUPLET: // not supported by importer yet
    default:
        LOGD("invalid note duration %d", static_cast<int>(duration));
        return 0; // invalid (result is weird layout)
    }
}
} // namespace mu::iex::tabledit
