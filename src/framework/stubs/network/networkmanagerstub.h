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
#ifndef MUSE_NETWORK_NETWORKMANAGERSTUB_H
#define MUSE_NETWORK_NETWORKMANAGERSTUB_H

#include "network/inetworkmanager.h"

namespace muse::network {
class NetworkManagerStub : public INetworkManager
{
public:
    Ret get(const QUrl& url, IncomingDevice* incomingData, const RequestHeaders& headers = RequestHeaders()) override;
    Ret head(const QUrl& url, const RequestHeaders& headers = RequestHeaders()) override;
    Ret post(const QUrl& url, OutgoingDevice* outgoingData, IncomingDevice* incomingData,
             const RequestHeaders& headers = RequestHeaders()) override;
    Ret put(const QUrl& url, OutgoingDevice* outgoingData, IncomingDevice* incomingData,
            const RequestHeaders& headers = RequestHeaders()) override;
    Ret patch(const QUrl& url, OutgoingDevice* outgoingData, IncomingDevice* incomingData,
              const RequestHeaders& headers = RequestHeaders()) override;
    Ret del(const QUrl& url, IncomingDevice* incomingData, const RequestHeaders& headers = RequestHeaders()) override;

    Progress progress() const override;

    void abort() override;
};
}

#endif // MUSE_NETWORK_NETWORKMANAGERSTUB_H
