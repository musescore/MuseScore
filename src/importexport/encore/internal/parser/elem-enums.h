/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

// Enumerations for Encore binary field values: format version, clef/staff/element/barline
// kinds, repeat marks, ornaments, accidentals, grace and text-alignment codes.

#pragma once

#include <QtGlobal>

namespace mu::iex::enc {
// Encore file format versions (byte at file offset 4).
enum class EncFormatVersion : quint8 {
    V2_X   = 0xA6,  // Encore 2.x (legacy)
    V3_4_X = 0xC2,  // Encore 3.x / 4.x
    V5_X   = 0xC4,  // Encore 5.x (current)
};

// Text encoding width of an instrument's strings: 1-byte (ANSI) or 2-byte (UTF-16 LE).
enum class EncCharSize : char {
    ONE_BYTE,
    TWO_BYTES
};

// Clef shape stored per staff/clef-change. ALIA (-1) = inherit/unchanged.
enum class EncClefType : qint8 {
    ALIA = -1,
    G    = 0,
    F    = 1,
    C3L  = 2,
    C4L  = 3,
    G8P  = 4,
    G8M  = 5,
    F8M  = 6,
    PERC = 7,
    TAB  = 8
};

// Staff notation kind: standard melody, guitar tablature, or rhythm-slash staff.
enum class EncStaffType : quint8 {
    MELODY  = 0,
    TAB     = 1,
    RHYTHM  = 2
};

enum class EncElemType : quint8 {
    NONE      = 0,
    CLEF      = 1,
    KEYCHANGE = 2,
    TIE       = 3,
    BEAM      = 4,
    ORNAMENT  = 5,
    LYRIC     = 6,
    CHORD     = 7,
    REST      = 8,
    NOTE      = 9,
    UNKNOWN1  = 10,
    MIDI_CC   = 11   // inline MIDI Control Change (sustain/volume/modulation); playback only, no notation
};

enum class EncBarlineType : quint8 {
    NORMAL      = 0,
    REPEATSTART = 2,
    DOUBLEL     = 3,
    REPEATEND   = 4,
    FINAL       = 5,
    DOUBLER     = 6,
    DOTTED      = 8
};

enum class EncRepeatType : quint8 {
    NONE     = 0,
    DCALCODA = 0x80,
    DSALCODA = 0x81,
    DCALFINE = 0x82,
    DSALFINE = 0x83,
    DS       = 0x84,
    CODA1    = 0x85,
    FINE     = 0x86,
    DC       = 0x87,
    SEGNO    = 0x88,
    CODA2    = 0x89
};

enum class EncOrnamentType : quint8 {
    // Lines, spanners, text
    NONE                   = 0,
    OTTAVA_ALTA            = 0x10,
    OTTAVA_BASSA           = 0x12,
    GRAPHIC_LINE           = 0x1C,
    WEDGESTART             = 0x1D,
    STAFFTEXT              = 0x1E,
    SLURSTART              = 0x21,
    ARPEGGIO               = 0x22,

    // Guitar bends, tempo
    GUITAR_BEND            = 0x28,
    GUITAR_BEND_2          = 0x29,
    GUITAR_PREBEND         = 0x2A,
    GUITAR_PREBEND_RELEASE = 0x2B,
    GUITAR_BEND_V          = 0x30,
    TEMPO                  = 0x32,

    // Trills, slur/wedge stops
    TRILL_END              = 0x35,
    TRILL_START            = 0x36,
    TRILL_ALT              = 0x37,
    SLURSTOP               = 0x41,
    WEDGESTOP              = 0x4D,

    // Dynamics
    DYN_PPP                = 0x80,
    DYN_PP                 = 0x81,
    DYN_P                  = 0x82,
    DYN_MP                 = 0x83,
    DYN_MF                 = 0x84,
    DYN_F                  = 0x85,
    DYN_FF                 = 0x86,
    DYN_FFF                = 0x87,
    DYN_SFZ                = 0x88,
    DYN_SFFZ               = 0x89,
    DYN_FP                 = 0x8A,

    // Navigation (segno/coda), breaths, more dynamics
    SEGNO                  = 0xA2,
    REPEAT_MEASURE         = 0xA3,
    TO_CODA                = 0xA5,
    CODA                   = 0xA6,
    CAESURA                = 0xA7,
    BREATH_COMMA           = 0xA8,
    DYN_FZ                 = 0xAA,
    DYN_SF                 = 0xAB,

    // Tremolo, trills, mordent, fingering, accents
    TREMOLO_32             = 0xAF,
    TRILL_TR               = 0xB0,
    TRILL_SHORT            = 0xB6,
    DOUBLE_MORDENT         = 0xB8,
    FINGER_1               = 0xB9,
    FINGER_2               = 0xBA,
    FINGER_3               = 0xBB,
    FINGER_4               = 0xBC,
    FINGER_5               = 0xBD,
    ACCENT                 = 0xBE,
    MARCATO                = 0xBF,
    MARCATO_STACCATO_BELOW = 0xC0,

    // Bows, marcato, tenuto, staccato, fermata
    UPBOW                  = 0xC4,
    DOWNBOW                = 0xC5,
    MARCATO_BELOW          = 0xC6,
    TENUTO                 = 0xC8,
    STACCATO               = 0xC9,
    FERMATA_ABOVE          = 0xCC,
    FERMATA_BELOW          = 0xCD,

    // String numbers, tremolo ladder
    STRING_NUMBER_2        = 0xE6,
    STRING_NUMBER_3        = 0xE7,
    STRING_NUMBER_4        = 0xE8,
    STRING_NUMBER_5        = 0xE9,
    STRING_NUMBER_6        = 0xEA,
    TREMOLO_16             = 0xEE,
    TREMOLO_32B            = 0xEF
};

enum class EncGraceType : char {
    NORMAL        = 0,
    ACCIACCATURA  = 1,
    APPOGGIATURA  = 2
};

// See ENCORE_FORMAT.md §TITL block for header/footer alignment byte values.
enum class EncTextAlign : quint8 {
    LEFT   = 0x04,
    CENTER = 0x06,
    RIGHT  = 0x02
};
} // namespace mu::iex::enc
