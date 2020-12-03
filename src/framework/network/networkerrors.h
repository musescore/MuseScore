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
#ifndef MU_FRAMEWORK_NETWORKERRORS_H
#define MU_FRAMEWORK_NETWORKERRORS_H

#include "ret.h"
#include "translation.h"

namespace mu {
namespace framework {
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::NetworkFirst),

    Abort,
    Timeout,
    NetworkError,
    HostClosed,
    HostNotFound,
    FiledOpenIODeviceRead,
    FiledOpenIODeviceWrite
};

inline Ret make_ret(Err e)
{
    int retCode = static_cast<int>(e);

    switch (e) {
    case Err::Undefined: return Ret(retCode);
    case Err::NoError: return Ret(retCode);
    case Err::UnknownError: return Ret(retCode);
    case Err::Abort: return Ret(retCode, trc("network", "The reply was aborted"));
    case Err::Timeout: return Ret(retCode, trc("network", "The connection to the remote server timed out"));
    case Err::NetworkError: return Ret(retCode, trc("network", "An unknown network-related error was detected"));
    case Err::HostClosed: return Ret(retCode, trc("network", "The remote server closed the connection"));
    case Err::HostNotFound: return Ret(retCode, trc("network", "The remote host name was not found"));
    case Err::FiledOpenIODeviceRead: return Ret(retCode, trc("network", "The I/O device was not opened for read"));
    case Err::FiledOpenIODeviceWrite: return Ret(retCode, trc("network", "The I/O device was not opened for write"));
    }

    return Ret(static_cast<int>(e));
}
}
}

#endif // MU_FRAMEWORK_NETWORKERRORS_H
