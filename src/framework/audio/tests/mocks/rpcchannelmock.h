/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <gmock/gmock.h>

#include "audio/common/rpc/irpcchannel.h"

namespace muse::audio::rpc {
class RpcChannelMock : public IRpcChannel
{
public:
    MOCK_METHOD(void, setupOnMain, (), (override));
    MOCK_METHOD(void, setupOnEngine, (), (override));

    MOCK_METHOD(void, process, (), (override));

    MOCK_METHOD(void, send, (const Msg&, const Handler&), (override));
    MOCK_METHOD(void, onMethod, (Method, Handler), (override));
    MOCK_METHOD(void, listenAll, (Handler), (override));

    MOCK_METHOD(void, addStream, (std::shared_ptr<IRpcStream>), (override));
    MOCK_METHOD(void, removeStream, (StreamId), (override));
    MOCK_METHOD(void, sendStream, (const StreamMsg&), (override));
    MOCK_METHOD(void, onStream, (StreamId, StreamHandler), (override));
};
}
