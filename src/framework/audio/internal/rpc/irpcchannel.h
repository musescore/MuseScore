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
#include "global/async/asyncable.h"

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
    AddTrackWithPlaybackData,
    AddTrackWithIODevice,
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

    GetSignalChanges,
    GetMasterSignalChanges,

    GetAvailableOutputResources,

    SaveSoundTrack,
    AbortSavingAllSoundTracks,

    ClearAllFx
};

inline std::string to_string(Method m)
{
    switch (m) {
    case Method::Undefined: return "Undefined";

    // Sequences
    case Method::AddSequence: return "AddSequence";
    case Method::RemoveSequence: return "RemoveSequence";
    case Method::GetSequenceIdList: return "GetSequenceIdList";

    // Tracks
    case Method::RemoveTrack: return "RemoveTrack";
    case Method::RemoveAllTracks: return "RemoveAllTracks";
    case Method::GetTrackIdList: return "GetTrackIdList";
    case Method::GetTrackName: return "GetTrackName";
    case Method::AddTrackWithPlaybackData: return "AddTrackWithPlaybackData";
    case Method::AddTrackWithIODevice: return "AddTrackWithIODevice";
    case Method::AddAuxTrack: return "AddAuxTrack";

    case Method::GetAvailableInputResources: return "GetAvailableInputResources";
    case Method::GetAvailableSoundPresets: return "GetAvailableSoundPresets";

    case Method::GetInputParams: return "GetInputParams";
    case Method::SetInputParams: return "SetInputParams";

    case Method::ClearSources: return "ClearSources";

    // Play
    case Method::Play: return "Play";
    case Method::Seek: return "Seek";
    case Method::Stop: return "Stop";
    case Method::Pause: return "Pause";
    case Method::Resume: return "Resume";
    case Method::SetDuration: return "SetDuration";
    case Method::SetLoop: return "SetLoop";
    case Method::ResetLoop: return "ResetLoop";

    // Output
    case Method::GetOutputParams: return "GetOutputParams";
    case Method::SetOutputParams: return "SetOutputParams";
    case Method::GetMasterOutputParams: return "GetMasterOutputParams";
    case Method::SetMasterOutputParams: return "SetMasterOutputParams";
    case Method::ClearMasterOutputParams: return "ClearMasterOutputParams";

    case Method::GetSignalChanges: return "GetSignalChanges";
    case Method::GetMasterSignalChanges: return "GetMasterSignalChanges";

    case Method::GetAvailableOutputResources: return "GetAvailableOutputResources";

    case Method::SaveSoundTrack: return "SaveSoundTrack";
    case Method::AbortSavingAllSoundTracks: return "AbortSavingAllSoundTracks";

    case Method::ClearAllFx: return "ClearAllFx";
    }

    assert(false && "unknown enum value");
    return std::to_string(static_cast<int>(m));
}

enum class MsgType {
    Undefined = 0,
    Notify,
    Request,
    Response
};

inline std::string to_string(MsgType t)
{
    switch (t) {
    case MsgType::Undefined: return "Undefined";
    case MsgType::Notify: return "Notify";
    case MsgType::Request: return "Request";
    case MsgType::Response: return "Response";
    }

    assert(false && "unknown enum value");
    return std::to_string(static_cast<int>(t));
}

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

enum class StreamType {
    Undefined = 0,
    Send,
    Receive
};

struct IRpcStream {
    virtual ~IRpcStream() = default;

    virtual StreamId streamId() const = 0;
    virtual StreamType type() const = 0;
    virtual void init() = 0;
    virtual bool inited() const = 0;
};

class IRpcChannel;
template<typename ... Types>
class RpcStream : public IRpcStream, public async::Asyncable
{
public:
    RpcStream(IRpcChannel* rpc, StreamId id, StreamType type, const async::Channel<Types...>& ch)
        : m_rpc(rpc), m_streamId(id), m_type(type), m_ch(ch) {}

    StreamId streamId() const override { return m_streamId; }
    StreamType type() const { return m_type; }
    void init() override;
    bool inited() const override { return m_inited; }

private:
    IRpcChannel* m_rpc = nullptr;
    StreamId m_streamId = 0;
    StreamType m_type = StreamType::Undefined;
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
    StreamId addSendStream(const async::Channel<Types...>& ch)
    {
        StreamId id = new_stream_id();
        std::shared_ptr<IRpcStream> s = std::shared_ptr<IRpcStream>(new RpcStream<Types...>(this, id, StreamType::Send, ch));
        addStream(s);
        return id;
    }

    template<typename ... Types>
    void addReceiveStream(rpc::StreamId id, const async::Channel<Types...>& ch)
    {
        std::shared_ptr<IRpcStream> s = std::shared_ptr<IRpcStream>(new RpcStream<Types...>(this, id, StreamType::Receive, ch));
        addStream(s);
    }

    virtual void addStream(std::shared_ptr<IRpcStream> s) = 0;
    virtual void removeStream(StreamId id) = 0;
    virtual void sendStream(const StreamMsg& msg) = 0;
    virtual void onStream(StreamId id, StreamHandler h) = 0;
};

template<typename ... Types>
void RpcStream<Types...>::init()
{
    if (m_inited) {
        return;
    }

    switch (m_type) {
    case StreamType::Send: {
        m_ch.onReceive(this, [this](const Types... args) {
                ByteArray data = RpcPacker::pack(args ...);
                m_rpc->sendStream(StreamMsg { m_streamId, data });
            });
    } break;
    case StreamType::Receive: {
        m_rpc->onStream(m_streamId, [this](const StreamMsg& msg) {
                std::tuple<Types...> values;
                bool success = std::apply([msg](auto&... args) {
                    return RpcPacker::unpack(msg.data, args ...);
                }, values);

                if (success) {
                    std::apply([this](const auto&... args) {
                        m_ch.send(args ...);
                    }, values);
                }
            });
    } break;
    case StreamType::Undefined: {
    } break;
    }

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
