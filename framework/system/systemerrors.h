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
#ifndef MU_FRAMEWORK_SYSTEMERRORS_H
#define MU_FRAMEWORK_SYSTEMERRORS_H

#include "ret.h"
#include "translation.h"

namespace mu {
namespace framework {
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::SystemFirst),

    FSNotExist,
    FSRemoveError,
    FSReadError,
    FSMakingError
};

inline Ret make_ret(Err e)
{
    int retCode = static_cast<int>(e);

    switch (e) {
    case Err::Undefined: return Ret(retCode);
    case Err::NoError: return Ret(retCode);
    case Err::UnknownError: return Ret(retCode);
    case Err::FSNotExist: return Ret(retCode, trc("system", "The file does not exist"));
    case Err::FSRemoveError: return Ret(retCode, trc("system", "The file could not be removed"));
    case Err::FSReadError: return Ret(retCode, trc("system", "An error occurred when reading from the file"));
    case Err::FSMakingError: return Ret(retCode, trc("system", "An error occurred when making a path"));
    }

    return Ret(static_cast<int>(e));
}
}
}

#endif // MU_FRAMEWORK_SYSTEMERRORS_H
