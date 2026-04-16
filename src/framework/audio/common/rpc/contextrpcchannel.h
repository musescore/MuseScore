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

#include "icontextrpcchannel.h"
#include "global/modularity/ioc.h"

namespace muse::audio::rpc {
class ContextRpcChannel : public IContextRpcChannel, public Contextable
{
    GlobalInject<IRpcChannel> globalChannel;
public:
    ContextRpcChannel(const muse::modularity::ContextPtr& ctx)
        : Contextable(ctx) {}

    // IContextRpcChannel
    void send(const Msg& msg, const Handler& onResponse = nullptr) override;
    void onMethod(Method method, Handler h) override;

    // IStreamRpcChannel
    void addStream(std::shared_ptr<IRpcStream> s) override;
    void removeStream(StreamId id) override;
    void sendStream(const StreamMsg& msg) override;
    void onStream(StreamId id, StreamHandler h) override;

private:

    inline CtxId contextId() const
    {
        assert(iocContext());
        assert(iocContext()->id != -1);
        if (!iocContext()) {
            return 0;
        }
        return static_cast<CtxId>(iocContext()->id);
    }
};
}
