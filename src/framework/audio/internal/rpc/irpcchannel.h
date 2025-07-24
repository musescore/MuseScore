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

#include <functional>

#include "global/modularity/imoduleinterface.h"

#include "global/types/bytearray.h"
#include "global/async/channel.h"

#include "rpcpacker.h"

namespace muse::audio::rpc {
using CallId = uint64_t;

enum class Method {
    Undefined = 0,

    // Sequences
    AddSequence,
    RemoveSequence,
    GetSequenceIdList,

    // Tracks
    RemoveTrack,
    RemoveAllTracks,
    GetTrackIdList,
    GetTrackName,
    AddAuxTrack,

    GetAvailableInputResources,
    GetAvailableSoundPresets,

    GetInputParams,
    SetInputParams,

    ClearSources,

    // Play
    Play,
    Seek,
    Stop,
    Pause,
    Resume,
    SetDuration,
    SetLoop,
    ResetLoop,

    // Output
    GetOutputParams,
    SetOutputParams,
    GetMasterOutputParams,
    SetMasterOutputParams,
    ClearMasterOutputParams,

    GetAvailableOutputResources,

    SaveSoundTrack,
    AbortSavingAllSoundTracks,

    ClearAllFx
};

enum class MsgType {
    Undefined = 0,
    Notify,
    Request,
    Response
};

struct Msg {
    CallId callId = 0;
    Method method = Method::Undefined;
    MsgType type = MsgType::Undefined;
    ByteArray data;
};

using Handler = std::function<void (const Msg& msg)>;

// stream
using StreamId = uint32_t;
inline StreamId new_stream_id()
{
    static StreamId lastId = 0;
    ++lastId;
    return lastId;
}

struct StreamMsg {
    StreamId streamId = 0;
    ByteArray data;
};

using StreamHandler = std::function<void (const StreamMsg& msg)>;

struct IStream {
    virtual ~IStream() = default;

    virtual StreamId streamId() const = 0;
    virtual void init() = 0;
    virtual bool inited() const = 0;
};

class IRpcChannel;
template<typename ... Types>
class Stream : public IStream
{
    Stream(IRpcChannel* rpc, StreamId id, const async::Channel<Types...>& ch)
        : m_rpc(rpc), m_streamId(id), m_ch(ch) {}

    StreamId streamId() const override
    {
        return m_streamId;
    }

    void init() override;

    bool inited() const
    {
        return m_inited;
    }

private:
    IRpcChannel* m_rpc = nullptr;
    StreamId m_streamId = 0;
    async::Channel<Types...> m_ch;
    bool m_inited = false;
};

class IRpcChannel : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IRpcChannel)
public:
    virtual ~IRpcChannel() = default;

    virtual void send(const Msg& msg, const Handler& onResponse = nullptr) = 0;
    virtual void onMethod(Method method, Handler h) = 0;
    virtual void listenAll(Handler h) = 0;

    // stream (async/channel)
    template<typename ... Types>
    StreamId addStream(const async::Channel<Types...>& ch)
    {
        StreamId id = new_stream_id();
        std::shared_ptr<IStream> s = std::shared_ptr<IStream>(new Stream<Types...>(id, ch));
        addStream(s);
        return id;
    }

    virtual void addStream(std::shared_ptr<IStream> s) = 0;
    virtual void removeStream(StreamId id) = 0;
    virtual void sendStream(const StreamMsg& msg) = 0;
    virtual void onStream(StreamId id, StreamHandler h) = 0;
};

template<typename ... Types>
void Stream<Types...>::init()
{
    if (m_inited) {
        return;
    }

    m_ch.onReceive(this, [this](const Types... args) {
        ByteArray data = msgpack::pack(args ...);
        m_rpc->sendStream(StreamMsg { m_streamId, data });
    });

    m_rpc->onStream(m_streamId, [this](const StreamMsg& msg) {
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

    m_inited = true;
}

// msgs
inline CallId new_call_id()
{
    static CallId lastId = 0;
    ++lastId;
    return lastId;
}

inline Msg make_request(Method m, const ByteArray& data = ByteArray())
{
    Msg msg;
    msg.callId = new_call_id();
    msg.method = m;
    msg.type = MsgType::Request;
    msg.data = data;
    return msg;
}

inline Msg make_response(const Msg& req, const ByteArray& data = ByteArray())
{
    Msg msg;
    msg.callId = req.callId;
    msg.method = req.method;
    msg.type = MsgType::Response;
    msg.data = data;
    return msg;
}
}
