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
static const std::string CLOUD_NETWORK_ERROR_USER_DESCRIPTION_KEY("userDescription");

enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::CloudFirst),

    AccessTokenIsEmpty,
    AccountNotActivated,
    Conflict,
    NetworkError, /// < use cloudNetworkErrorUserDescription to retrieve user-readable description
    CouldNotReceiveSourceUrl,
};

inline Ret make_ret(Err e)
{
    int retCode = static_cast<int>(e);

    switch (e) {
    case Err::Undefined: return Ret(retCode);
    case Err::NoError: return Ret(retCode);
    case Err::UnknownError: return Ret(retCode);
    case Err::AccessTokenIsEmpty: return Ret(retCode, "Access token is empty");
    case Err::AccountNotActivated: return Ret(retCode, "Account not activated");
    case Err::Conflict: return Ret(retCode, "Conflict");
    case Err::NetworkError: return Ret(retCode, "Network error");
    case Err::CouldNotReceiveSourceUrl: return Ret(retCode, "Could not receive source url");
    }

    return Ret(static_cast<int>(e));
}

inline std::string cloudNetworkErrorUserDescription(const Ret& ret)
{
    assert(ret.code() == int(Err::NetworkError));

    auto userDescription = ret.data(CLOUD_NETWORK_ERROR_USER_DESCRIPTION_KEY);
    if (userDescription.has_value()) {
        return std::any_cast<std::string>(userDescription);
    }

    return {};
}
}

#endif // MU_CLOUD_CLOUDERRORS_H
