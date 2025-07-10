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
#ifndef MUSE_AUDIO_AUDIOERRORS_H
#define MUSE_AUDIO_AUDIOERRORS_H

#include "global/types/ret.h"

namespace muse::audio {
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
    SynthNotInited = 330,
    NoLoadedSoundFonts = 331,
    SoundFontFailedLoad = 332,
    SoundFontFailedUnload = 333,
    UnknownSynthType = 334,

    // common
    InvalidTrackId = 340,
    InvalidMixerChannelId = 341,
    InvalidSequenceId = 342,
    InvalidSetupData = 343,
    InvalidAudioSource = 344,
    InvalidAudioFilePath = 345,
    InvalidFxParams = 346,
    InvalidAudioSourceParams = 347,
    DisabledAudioExport = 348,
    NoAudioToExport = 349,
    ErrorEncode = 350,
    UnknownPluginType = 351,

    // clock
    InvalidTimeLoop = 360,

    // online sounds
    OnlineSoundsNetworkError = 361,
};

inline Ret make_ret(Err e)
{
    return Ret(static_cast<int>(e));
}
}

#endif // MUSE_AUDIO_AUDIOERRORS_H
