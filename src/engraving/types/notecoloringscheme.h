/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
 */

/*!
 * \file
 * \brief Note coloring schemes and pitch-class helpers for engraving and Edit Style.
 *
 * Defines @c NoteColoringScheme and related pitch-class helpers used by @c Note coloring.
 */
#pragma once

namespace mu::engraving {
/*!
 * Style-driven scheme for mapping notes to palette swatches (@c Sid::noteColor0 ... @c Sid::noteColor11).
 */
enum class NoteColoringScheme : int {
    OneColor = 0, //!< Single user color for all notes.
    AbsolutePitchSimple = 1,      //!< Step index from TPC (letter name / diatonic step).
    AbsolutePitchChromatic = 2,   //!< MIDI chroma from effective pitch.
    MoveableDoSimple = 3,         //!< Scale degree from TPC and local key.
    MoveableDoChromatic = 4,      //!< Chromatic scale degree relative to key tonic.
};

/*! Semitone offsets of the major scale from the tonic (pitch classes). */
inline constexpr int MAJOR_SCALE_INTERVALS[] = { 0, 2, 4, 5, 7, 9, 11 };

/*!
 * Pitch class (0-11) of the major tonic for a key signature given as circle-of-fifths index.
 * @param keyFifths @c Key enum value (fifths from C).
 * @return Tonic pitch class 0-11.
 */
inline int tonicPitchClassFromKey(int keyFifths)
{
    int pc = (keyFifths * 7) % 12;
    return pc < 0 ? pc + 12 : pc;
}

/*!
 * Diatonic degree index 0-6 for @p pitchClass in a major scale built on @p tonicPC, or -1 if not in scale.
 * @param pitchClass Note chroma 0-11.
 * @param tonicPC Major-key tonic chroma 0-11.
 * @return Major scale degree index, or -1.
 */
inline int pitchToDegreeIndex(int pitchClass, int tonicPC)
{
    int semitones = (pitchClass - tonicPC + 12) % 12;
    for (int i = 0; i < 7; ++i) {
        if (MAJOR_SCALE_INTERVALS[i] == semitones) {
            return i;
        }
    }
    return -1;
}
}
