/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef MU_ENGRAVING_PITCHUTILS_H
#define MU_ENGRAVING_PITCHUTILS_H

#include "mpe/mpetypes.h"

#include "libmscore/pitchspelling.h"

namespace mu::engraving {
inline mpe::PitchClass pitchClassFromTpc(const int tpc)
{
    switch (tpc) {
    // C
    case TPC_D_BB:
    case TPC_C:
    case TPC_B_S:
    case TPC_A_SSS:
        return mpe::PitchClass::C;
    case TPC_E_BBB:
    case TPC_D_B:
    case TPC_C_S:
    case TPC_B_SS:
        return mpe::PitchClass::C_sharp;

    // D
    case TPC_F_BBB:
    case TPC_E_BB:
    case TPC_D:
    case TPC_C_SS:
    case TPC_B_SSS:
        return mpe::PitchClass::D;
    case TPC_F_BB:
    case TPC_E_B:
    case TPC_D_S:
    case TPC_C_SSS:
        return mpe::PitchClass::D_sharp;

    // E
    case TPC_G_BBB:
    case TPC_F_B:
    case TPC_E:
    case TPC_D_SS:
        return mpe::PitchClass::E;

    // F
    case TPC_G_BB:
    case TPC_F:
    case TPC_E_S:
    case TPC_D_SSS:
        return mpe::PitchClass::F;
    case TPC_A_BBB:
    case TPC_G_B:
    case TPC_F_S:
    case TPC_E_SS:
        return mpe::PitchClass::F_sharp;

    // G
    case TPC_A_BB:
    case TPC_G:
    case TPC_F_SS:
    case TPC_E_SSS:
        return mpe::PitchClass::G;
    case TPC_B_BBB:
    case TPC_A_B:
    case TPC_G_S:
    case TPC_F_SSS:
        return mpe::PitchClass::G_sharp;

    // A
    case TPC_C_BBB:
    case TPC_B_BB:
    case TPC_A:
    case TPC_G_SS:
        return mpe::PitchClass::A;
    case TPC_C_BB:
    case TPC_B_B:
    case TPC_A_S:
    case TPC_G_SSS:
        return mpe::PitchClass::A_sharp;

    // B
    case TPC_D_BBB:
    case TPC_C_B:
    case TPC_B:
    case TPC_A_SS:
        return mpe::PitchClass::B;

    default:
        return mpe::PitchClass::Undefined;
    }
}

inline mpe::octave_t actualOctave(const int nominalOctave, const mpe::PitchClass nominalPitchClass,
                                  const AccidentalVal accidental)
{
    int shift = static_cast<int>(nominalPitchClass) - static_cast<int>(accidental);

    constexpr int lowerBound = 0;
    constexpr int upperBound = static_cast<int>(mpe::PitchClass::Last) - 1;

    if (shift < lowerBound) {
        return static_cast<mpe::octave_t>(nominalOctave + 1);
    }

    if (shift > upperBound) {
        return static_cast<mpe::octave_t>(nominalOctave - 1);
    }

    return static_cast<mpe::octave_t>(nominalOctave);
}

inline mpe::pitch_level_t notePitchLevel(const int noteTpc, const int noteOctave, const double tuningCents = 0.0)
{
    mpe::PitchClass pitchClass = pitchClassFromTpc(noteTpc);

    double tuningFactor = tuningCents / 100.0;

    mpe::pitch_level_t result = mpe::pitchLevel(pitchClass, actualOctave(noteOctave, pitchClass, tpc2alter(noteTpc)));
    result += tuningFactor * mpe::PITCH_LEVEL_STEP;

    return result;
}
}

#endif // MU_ENGRAVING_PITCHUTILS_H
