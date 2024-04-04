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

#ifndef MUSE_CLOUD_CLOUDERRORS_H
#define MUSE_CLOUD_CLOUDERRORS_H

#include "types/ret.h"

namespace muse::cloud {
static const std::string CLOUD_NETWORK_ERROR_USER_DESCRIPTION_KEY("userDescription");

enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::CloudFirst),

    AccessTokenIsEmpty,
    Status400_InvalidRequest,
    Status401_AuthorizationRequired,
    Status403_AccountNotActivated,
    Status403_NotOwner,
    Status404_NotFound,
    Status409_Conflict,
    Status422_ValidationFailed,
    Status429_RateLimitExceeded,
    Status500_InternalServerError,
    UnknownStatusCode,
    NetworkError,
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
    case Err::Status400_InvalidRequest: return Ret(retCode, "Status 400: invalid request");
    case Err::Status401_AuthorizationRequired: return Ret(retCode, "Status 401: authorization required");
    case Err::Status403_AccountNotActivated: return Ret(retCode, "Status 403: account not activated");
    case Err::Status403_NotOwner: return Ret(retCode, "Status 403: not owner");
    case Err::Status404_NotFound: return Ret(retCode, "Status 404: not found");
    case Err::Status409_Conflict: return Ret(retCode, "Status 409: conflict");
    case Err::Status422_ValidationFailed: return Ret(retCode, "Status 422: validation failed");
    case Err::Status429_RateLimitExceeded: return Ret(retCode, "Status 429: rate limit exceeded");
    case Err::Status500_InternalServerError: return Ret(retCode, "Status 500: internal server error");
    case Err::UnknownStatusCode: return Ret(retCode, "Unknown status code");
    case Err::NetworkError: return Ret(retCode, "Network error");
    case Err::CouldNotReceiveSourceUrl: return Ret(retCode, "Could not receive source url");
        break;
    }

    return Ret(retCode);
}
}

#endif // MUSE_CLOUD_CLOUDERRORS_H
