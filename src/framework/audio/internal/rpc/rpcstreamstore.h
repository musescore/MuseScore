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

#include <map>

#include "rpcstream.h"

namespace muse::audio::rpc {
class RpcStreamStore
{
public:

    template<typename ... Types>
    static StreamId newRpcStream(const async::Channel<Types...>& ch)
    {
        StreamId id = new_stream_id();
        IStream* s = new Stream<Types...>(id, ch);
        m_streams.insert({ id, s });
        return id;
    }

    template<typename ... Types>
    static StreamId newRpcStream(StreamId id, const async::Channel<Types...>& ch)
    {
        IStream* s = new Stream<Types...>(id, ch);
        m_streams.insert({ id, s });
        return id;
    }

    static void removeStream(StreamId id)
    {
        auto it = m_streams.find(id);
        if (it != m_streams.end()) {
            delete it->second;
            m_streams.erase(it);
        }
    }

private:

    struct IStream {
        virtual ~IStream() = default;
    };

    template<typename ... Types>
    struct Stream : public IStream {
        RpcStream<Types...> stream;
        Stream(StreamId id, const async::Channel<Types...>& ch)
            : stream(id, ch) {}
    };

    static std::map<StreamId, IStream*> m_streams;
};
}
