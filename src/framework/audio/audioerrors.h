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
#ifndef MU_AUDIO_AUDIOERRORS_H
#define MU_AUDIO_AUDIOERRORS_H

#include "ret.h"

namespace mu::audio {
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::AudioFirst),

    // driver
    DriverNotFound = 301,
    DriverOpenFailed = 302,

    // engine
    EngineInvalidParameter = 310,

    AudioStreamNotPresent = 320,

    // synth
    SynthNotInited = 331,
    SoundFontNotLoaded = 332,
    SoundFontFailedLoad = 333,
    SoundFontFailedUnload = 334,

    //common
    InvalidTrackId = 335,
    InvalidMixerChannelId = 336,
    InvalidSequenceId = 337,
    InvalidMidiMapping = 338,
    InvalidAudioSource = 339,
    InvalidAudioFilePath = 340,

    // clock
    InvalidTimeLoop = 341,
};

inline Ret make_ret(Err e)
{
    return Ret(static_cast<int>(e));
}
}

#endif // MU_AUDIO_AUDIOERRORS_H
