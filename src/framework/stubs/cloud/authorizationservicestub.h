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
#ifndef MU_CLOUD_AUTHORIZATIONSERVICESTUB_H
#define MU_CLOUD_AUTHORIZATIONSERVICESTUB_H

#include "cloud/iauthorizationservice.h"

namespace muse::cloud {
class AuthorizationServiceStub : public IAuthorizationService
{
public:
    void signUp() override;
    void signIn() override;
    void signOut() override;

    RetVal<Val> ensureAuthorization(bool publishingScore, const std::string& text = {}) override;

    ValCh<bool> userAuthorized() const override;
    ValCh<AccountInfo> accountInfo() const override;

    CloudInfo cloudInfo() const override;

    Ret checkCloudIsAvailable() const override;
};
}

#endif // MU_CLOUD_AUTHORIZATIONSERVICESTUB_H
