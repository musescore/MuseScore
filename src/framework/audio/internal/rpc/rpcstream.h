/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "global/async/asyncable.h"
#include "global/async/channel.h"

#include "modularity/ioc.h"
#include "irpcchannel.h"

#include "global/serialization/msgpack.h"

namespace muse::audio::rpc {
//! NOTE This is an adapter to the async channel
template<typename ... Types>
class RpcStream : public async::Asyncable
{
    Inject<rpc::IRpcChannel> rpc;

public:
    RpcStream(async::Channel<Types...> ch)
        : m_ch(ch)
    {
        m_streamId = new_stream_id();

        m_ch.onReceive(this, [this](const Types... args) {
            ByteArray data = msgpack::pack(args ...);
            rpc()->sendStream(StreamMsg { m_streamId, data });
        });

        rpc()->onStream(m_streamId, [this](const StreamMsg& msg) {
            std::tuple<Types...> values;
            bool success = std::apply([msg](auto&... args) {
                return msgpack::unpack(msg.data, args ...);
            }, values);

            if (success) {
                std::apply([this](const auto&... args) {
                    m_ch.send(args ...);
                }, values);
            }
        });
    }

private:
    StreamId m_streamId = 0;
    async::Channel<Types...> m_ch;
};
}
