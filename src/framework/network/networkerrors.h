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
#ifndef MUSE_NETWORK_NETWORKERRORS_H
#define MUSE_NETWORK_NETWORKERRORS_H

#include "types/ret.h"
#include "translation.h"

namespace muse::network {
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::NetworkFirst),

    Abort,
    Timeout,
    NetworkError,
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
    case Err::Abort: return Ret(retCode, muse::trc("network", "The request was aborted"));
    case Err::Timeout: return Ret(retCode, muse::trc("network", "The connection to the remote server timed out"));
    case Err::NetworkError: return Ret(retCode, muse::trc("network", "An unknown network-related error occurred"));
    case Err::FiledOpenIODeviceRead: return Ret(retCode, muse::trc("network", "The I/O device was not opened for reading"));
    case Err::FiledOpenIODeviceWrite: return Ret(retCode, muse::trc("network", "The I/O device was not opened for writing"));
    }

    return Ret(static_cast<int>(e));
}
}

#endif // MUSE_NETWORK_NETWORKERRORS_H
