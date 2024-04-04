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
#ifndef MUSE_CLOUD_IAUTHORIZATIONSERVICE_H
#define MUSE_CLOUD_IAUTHORIZATIONSERVICE_H

#include "modularity/imoduleinterface.h"
#include "cloudtypes.h"

#include "types/val.h"
#include "types/retval.h"

namespace muse::cloud {
class IAuthorizationService : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAuthorizationService)

public:
    virtual ~IAuthorizationService() = default;

    virtual void signUp() = 0;
    virtual void signIn() = 0;
    virtual void signOut() = 0;

    virtual RetVal<Val> ensureAuthorization(bool publishingScore, const std::string& text = {}) = 0;

    virtual ValCh<bool> userAuthorized() const = 0;
    virtual ValCh<AccountInfo> accountInfo() const = 0;

    virtual CloudInfo cloudInfo() const = 0;

    virtual Ret checkCloudIsAvailable() const = 0;
};
using IAuthorizationServicePtr = std::shared_ptr<IAuthorizationService>;
}

#endif // MUSE_CLOUD_IAUTHORIZATIONSERVICE_H
