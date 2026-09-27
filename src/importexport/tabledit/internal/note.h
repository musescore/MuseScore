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

// note attribute voice
enum class Voice : uint8_t {
    DEFAULT = 0,    // default: none set
    UPPER = 2,      // upper set
    LOWER = 3       // lower set
};

struct TefNote {
    int position { 0 };
    int string { 0 };
    int fret { 0 };
    bool tie { false };
    bool rest { false };    // this is a bit of a hack
    int duration { 0 };     // this is the duration as encoded in the .tef file
    int length { 0 };
    int dots { 0 };
    bool triplet { false };
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

int durationToInt(uint8_t duration);
} // namespace mu::iex::tabledit
