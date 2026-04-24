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

#include <map>

#include "global/modularity/ioc.h"
#include "irpcchannel.h"

#include "icontextrpcchannel.h"

namespace muse::audio::rpc {
class ContextRpcChannelController : public IContextRpcChannelController
{
    GlobalInject<IRpcChannel> globalChannel;

public:
    ContextRpcChannelController() = default;

    void send(const Msg& msg, const Handler& onResponse = nullptr) override;
    void onRequest(CtxId ctxId, MsgCode code, Handler h) override;
    void onNotification(CtxId ctxId, MsgCode code, Handler h) override;

    //! NOTE Streams have unique IDs, so there is no need use context ID for them.
    void addStream(std::shared_ptr<IRpcStream> s) override;
    void removeStream(StreamId id) override;
    void sendStream(const StreamMsg& msg) override;
    void onStream(StreamId id, StreamHandler h) override;

private:

    std::map<MsgCode, std::map<CtxId, Handler> > m_onRequests;
    std::map<MsgCode, std::map<CtxId, Handler> > m_onNotifications;
};
}
