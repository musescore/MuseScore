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
    case Ms::TPC_D_BB:
    case Ms::TPC_C:
    case Ms::TPC_B_S:
    case Ms::TPC_A_SSS:
        return mpe::PitchClass::C;
    case Ms::TPC_E_BBB:
    case Ms::TPC_D_B:
    case Ms::TPC_C_S:
    case Ms::TPC_B_SS:
        return mpe::PitchClass::C_sharp;

    // D
    case Ms::TPC_F_BBB:
    case Ms::TPC_E_BB:
    case Ms::TPC_D:
    case Ms::TPC_C_SS:
    case Ms::TPC_B_SSS:
        return mpe::PitchClass::D;
    case Ms::TPC_F_BB:
    case Ms::TPC_E_B:
    case Ms::TPC_D_S:
    case Ms::TPC_C_SSS:
        return mpe::PitchClass::D_sharp;

    // E
    case Ms::TPC_G_BBB:
    case Ms::TPC_F_B:
    case Ms::TPC_E:
    case Ms::TPC_D_SS:
        return mpe::PitchClass::E;

    // F
    case Ms::TPC_G_BB:
    case Ms::TPC_F:
    case Ms::TPC_E_S:
    case Ms::TPC_D_SSS:
        return mpe::PitchClass::F;
    case Ms::TPC_A_BBB:
    case Ms::TPC_G_B:
    case Ms::TPC_F_S:
    case Ms::TPC_E_SS:
        return mpe::PitchClass::F_sharp;

    // G
    case Ms::TPC_A_BB:
    case Ms::TPC_G:
    case Ms::TPC_F_SS:
    case Ms::TPC_E_SSS:
        return mpe::PitchClass::G;
    case Ms::TPC_B_BBB:
    case Ms::TPC_A_B:
    case Ms::TPC_G_S:
    case Ms::TPC_F_SSS:
        return mpe::PitchClass::G_sharp;

    // A
    case Ms::TPC_C_BBB:
    case Ms::TPC_B_BB:
    case Ms::TPC_A:
    case Ms::TPC_G_SS:
        return mpe::PitchClass::A;
    case Ms::TPC_C_BB:
    case Ms::TPC_B_B:
    case Ms::TPC_A_S:
    case Ms::TPC_G_SSS:
        return mpe::PitchClass::A_sharp;

    // B
    case Ms::TPC_D_BBB:
    case Ms::TPC_C_B:
    case Ms::TPC_B:
    case Ms::TPC_A_SS:
        return mpe::PitchClass::B;

    default:
        return mpe::PitchClass::Undefined;
    }
}

inline mpe::octave_t actualOctave(const int nominalOctave, const mpe::PitchClass nominalPitchClass, const Ms::AccidentalVal accidental)
{
    int shift = static_cast<int>(nominalPitchClass) - static_cast<int>(accidental);

    constexpr int lowerBound = 0;
    constexpr int upperBound = static_cast<int>(mpe::PitchClass::Last) - 1;

    if (shift < lowerBound) {
        return nominalOctave + 1;
    }

    if (shift > upperBound) {
        return nominalOctave - 1;
    }

    return nominalOctave;
}

inline mpe::pitch_level_t notePitchLevel(const int noteTpc, const int noteOctave)
{
    mpe::PitchClass pitchClass = pitchClassFromTpc(noteTpc);

    return mpe::pitchLevel(pitchClass, actualOctave(noteOctave, pitchClass, Ms::tpc2alter(noteTpc)));
}
}

#endif // MU_ENGRAVING_PITCHUTILS_H
