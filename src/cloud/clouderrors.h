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

#ifndef MU_CLOUD_CLOUDERRORS_H
#define MU_CLOUD_CLOUDERRORS_H

#include "types/ret.h"

namespace mu::cloud {
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::CloudFirst),

    UserIsNotAuthorized,
    AccessTokenIsEmpty,
    CouldNotReceiveSourceUrl,
};

inline Ret make_ret(Err e)
{
    int retCode = static_cast<int>(e);

    switch (e) {
    case Err::Undefined: return Ret(retCode);
    case Err::NoError: return Ret(retCode);
    case Err::UnknownError: return Ret(retCode);
    case Err::UserIsNotAuthorized: return Ret(retCode, "User is not authorized");
    case Err::AccessTokenIsEmpty: return Ret(retCode, "Access token is empty");
    case Err::CouldNotReceiveSourceUrl: return Ret(retCode, "Could not receive source url");
    }

    return Ret(static_cast<int>(e));
}
}

#endif // MU_CLOUD_CLOUDERRORS_H
