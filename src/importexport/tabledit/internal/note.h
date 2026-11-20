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
#pragma once

namespace mu::iex::tabledit {
enum class EffectType : int8_t {
    INVALID = -1,
    NONE,
    HAMMER_ON,
    PULL_OFF,
    SLIDE,
    CHOKE,
    BRUSH,
    NATURAL_HARMONIC,
    ARTIFICIAL_HARMONIC,
    MUTED,
    TAPPING,
    VIBRATO,
    TREMOLO,
    SIMPLE_BEND,
    BEND_AND_RELEASE,
    ROLL_ARPEGGIO,
    DEAD_NOTE,
    RINGING_NOTE,
    SLAP,
    RASGUEADO,
    GHOST_NOTE,
    TREMOLO_UP_DOWN,
    TREMOLO_DIVE_RETURN,
    STACCATO,
    FADE_IN,
    FADE_OUT,
    ROLL,   // specific for combination effect
    VARIATION
};

/*
 * TablEdit encoding of note duration is an enum with 32 possible values,
 * some of which are not used. It encodes note type, number of dots
 * and tuplet type.
 */

enum class NoteDuration : uint8_t {
    V_WHOLE =  0,
    V_HALF_DOTTED =  1,
    V_WHOLE_TRIPLET =  2,
    V_HALF =  3,
    V_QUARTER_DOTTED =  4,
    V_HALF_TRIPLET =  5,
    V_QUARTER =  6,
    V_EIGHTH_DOTTED =  7,
    V_QUARTER_TRIPLET =  8,
    V_EIGHTH =  9,
    V_16TH_DOTTED = 10,
    V_EIGHTH_TRIPLET = 11,
    V_16TH = 12,
    V_32ND_DOTTED = 13,
    V_16TH_TRIPLET = 14,
    V_32ND = 15,
    V_64TH_DOTTED = 16,
    V_32ND_TRIPLET = 17,
    V_64TH = 18,
    V_HALF_DOUBLE_DOTTED = 19,
    V_16TH_QUINTUPLET = 20,
    V_RESERVED_1 = 21,
    V_QUARTER_DOUBLE_DOTTED = 22,
    V_32ND_SEPTUPLET = 23,
    V_RESERVED_2 = 24,
    V_EIGHTH_DOUBLE_DOTTED = 25,
    V_32ND_NONUPLET = 26,
    V_RESERVED_3 = 27,
    V_16TH_DOUBLE_DOTTED = 28,
    V_RESERVED_4 = 29,
    V_RESERVED_5 = 30,
    V_WHOLE_DOTTED = 31
};

// note attribute voice
enum class Voice : uint8_t {
    DEFAULT = 0,    // default: none set
    UPPER = 2,      // upper set
    LOWER = 3       // lower set
};

constexpr static int WHOLE_DIVISION = 64; // TablEdit calculates in 64th

struct TefNote {
    int position { 0 };
    int string { 0 };
    int fret { 0 };
    bool tie { false };
    bool rest { false };    // this is a bit of a hack
    NoteDuration duration { 0 };     // duration as encoded in the .tef file
    int type() const;   // note type in 64th
    int dots() const;   // number of dots
    bool tuplet() const;
    int lengthInclDots() const;
    int tablEditLength() const;
    Voice voice { 0 };
    bool hasGrace { false };
    int graceEffect{ -1 };  // invalid
    int graceFret { -1 };   // invalid
    int fingeringLH { 0 };
    int fingeringRH { 0 };
    int simpleEffect { 0 };
    int complexEffect { 0 };
    EffectType effect() const;
    EffectType combinationEffect() const;
};
} // namespace mu::iex::tabledit
