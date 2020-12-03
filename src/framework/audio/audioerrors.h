//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_AUDIO_AUDIOERRORS_H
#define MU_AUDIO_AUDIOERRORS_H

#include "ret.h"

namespace mu {
namespace audio {
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::AudioFirst),

    // driver
    DriverNotFound = 301,
    DriverOpenFailed = 302,

    // engine
    EngineInvalidParameter = 310,
};

inline Ret make_ret(Err e)
{
    return Ret(static_cast<int>(e));
}
}
}

#endif // MU_AUDIO_AUDIOERRORS_H
