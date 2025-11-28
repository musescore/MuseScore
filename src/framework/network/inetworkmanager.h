/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#pragma once

#include "global/types/ret.h"
#include "global/progress.h"

#include "networktypes.h"

class QUrl;

namespace muse::network {
class INetworkManager
{
public:
    virtual ~INetworkManager() = default;

    virtual RetVal<Progress> get(const QUrl& url, IncomingDevicePtr incomingData, const RequestHeaders& headers = RequestHeaders()) = 0;
    virtual RetVal<Progress> head(const QUrl& url, const RequestHeaders& headers = RequestHeaders()) = 0;
    virtual RetVal<Progress> post(const QUrl& url, OutgoingDeviceVar outgoingData, IncomingDevicePtr incomingData,
                                  const RequestHeaders& headers = RequestHeaders()) = 0;
    virtual RetVal<Progress> put(const QUrl& url, OutgoingDeviceVar outgoingData, IncomingDevicePtr incomingData,
                                 const RequestHeaders& headers = RequestHeaders()) = 0;
    virtual RetVal<Progress> patch(const QUrl& url, OutgoingDeviceVar outgoingData, IncomingDevicePtr incomingData,
                                   const RequestHeaders& headers = RequestHeaders()) = 0;
    virtual RetVal<Progress> del(const QUrl& url, IncomingDevicePtr incomingData, const RequestHeaders& headers = RequestHeaders()) = 0;
};

using INetworkManagerPtr = std::shared_ptr<INetworkManager>;
}
