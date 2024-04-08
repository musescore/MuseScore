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
#ifndef MUSE_NETWORK_INETWORKMANAGER_H
#define MUSE_NETWORK_INETWORKMANAGER_H

#include "types/ret.h"
#include "global/progress.h"
#include "networktypes.h"

class QUrl;

namespace muse::network {
class INetworkManager
{
public:
    virtual ~INetworkManager() = default;

    virtual Ret get(const QUrl& url, IncomingDevice* incomingData, const RequestHeaders& headers = RequestHeaders()) = 0;
    virtual Ret head(const QUrl& url, const RequestHeaders& headers = RequestHeaders()) = 0;
    virtual Ret post(const QUrl& url, OutgoingDevice* outgoingData, IncomingDevice* incomingData,
                     const RequestHeaders& headers = RequestHeaders()) = 0;
    virtual Ret put(const QUrl& url, OutgoingDevice* outgoingData, IncomingDevice* incomingData,
                    const RequestHeaders& headers = RequestHeaders()) = 0;
    virtual Ret patch(const QUrl& url, OutgoingDevice* outgoingData, IncomingDevice* incomingData,
                      const RequestHeaders& headers = RequestHeaders()) = 0;
    virtual Ret del(const QUrl& url, IncomingDevice* incomingData, const RequestHeaders& headers = RequestHeaders()) = 0;

    virtual Progress progress() const = 0;

    virtual void abort() = 0;
};

using INetworkManagerPtr = std::shared_ptr<INetworkManager>;
}

#endif // MUSE_NETWORK_INETWORKMANAGER_H
