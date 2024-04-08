/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#ifndef MUSE_NETWORK_NETWORKMANAGERMOCK_H
#define MUSE_NETWORK_NETWORKMANAGERMOCK_H

#include <gmock/gmock.h>

#include "network/inetworkmanager.h"

namespace muse::network {
class NetworkManagerMock : public INetworkManager
{
public:
    MOCK_METHOD(Ret, get, (const QUrl&, IncomingDevice*, const RequestHeaders&), (override));
    MOCK_METHOD(Ret, head, (const QUrl&, const RequestHeaders&), (override));
    MOCK_METHOD(Ret, post, (const QUrl&, OutgoingDevice*, IncomingDevice*, const RequestHeaders&), (override));
    MOCK_METHOD(Ret, put, (const QUrl&, OutgoingDevice*, IncomingDevice*, const RequestHeaders&), (override));
    MOCK_METHOD(Ret, patch, (const QUrl&, OutgoingDevice*, IncomingDevice*, const RequestHeaders&), (override));
    MOCK_METHOD(Ret, del, (const QUrl&, IncomingDevice*, const RequestHeaders&), (override));

    MOCK_METHOD(Progress, progress, (), (const, override));

    MOCK_METHOD(void, abort, (), (override));
};
}

#endif // MUSE_NETWORK_NETWORKMANAGERMOCK_H
