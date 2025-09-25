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
#include "webrpcchannel.h"

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include "global/serialization/msgpack.h"

#include "log.h"

//#define RPC_LOGGING_ENABLED

#ifdef RPC_LOGGING_ENABLED
#define RPCLOG() LOGDA()
#else
#define RPCLOG() LOGN()
#endif

using namespace muse;
using namespace muse::audio::rpc;

static constexpr size_t DEFAULT_CAPACITY = 1024 * 200;
static std::vector<uint8_t> buffer = {};
static std::function<void(const ByteArray&)> g_rpcListen = nullptr;

static constexpr int MSG_ID = 1;
static constexpr int STREAM_ID = 2;

static void rpcSend(const uint8_t* data, size_t size)
{
    emscripten::val jsArray = emscripten::val::global("Uint8Array").new_(
        emscripten::typed_memory_view(size, data)
        );

    emscripten::val::module_property("main_worker_rpcSend")(jsArray);
}

static void rpcListen(const emscripten::val& data)
{
    IF_ASSERT_FAILED(g_rpcListen) {
        return;
    }

    IF_ASSERT_FAILED(data.instanceof(emscripten::val::global("Uint8Array"))) {
        return;
    }

    size_t length = data["length"].as<size_t>();
    ByteArray ba(length);

    emscripten::val memoryView = emscripten::val(emscripten::typed_memory_view(
                                                     ba.size(),
                                                     ba.data()
                                                     ));
    memoryView.call<void>("set", data);

    g_rpcListen(ba);
}

EMSCRIPTEN_BINDINGS(RpcChannel) {
    function("main_worker_rpcListen", &rpcListen);
}

void WebRpcChannel::setupOnMain()
{
    g_rpcListen = [this](const ByteArray& d) {
        receive(d);
    };
}

void WebRpcChannel::setupOnEngine()
{
    g_rpcListen = [this](const ByteArray& d) {
        receive(d);
    };
}

void WebRpcChannel::process()
{
    //noop
}

void WebRpcChannel::send(const Msg& msg, const Handler& onResponse)
{
    RPCLOG() << "callId: " << msg.callId
             << ", method: " << to_string(msg.method)
             << ", type: " << to_string(msg.type)
             << ", data.size: " << msg.data.size();

    // clear but keep capacity
    buffer.clear();
    // makes sense if the reserve is greater than the current capacity
    buffer.reserve(std::max(msg.data.size(), DEFAULT_CAPACITY));

    msgpack::pack(buffer, MSG_ID, msg.callId, (int)msg.method, (int)msg.type, msg.data.constVData());

    IF_ASSERT_FAILED(buffer.size() > 0) {
        return;
    }

    if (onResponse) {
        m_data.onResponses[msg.callId] = onResponse;
    }

    rpcSend(&buffer[0], buffer.size());
}

void WebRpcChannel::receive(const ByteArray& data)
{
    IF_ASSERT_FAILED(data.size() > 0) {
        return;
    }
    muse::msgpack::Cursor cursor(data.constData(), data.size());
    int msgId = 0;
    msgpack::unpack(cursor, msgId);

    if (msgId == MSG_ID) {
        Msg msg;
        int method = 0;
        int type = 0;
        msgpack::unpack(cursor, msg.callId, method, type, msg.data.vdata());
        msg.method = static_cast<Method>(method);
        msg.type = static_cast<MsgType>(type);

        receive(msg);
    } else if (msgId == STREAM_ID) {
        StreamMsg msg;
        int name = 0;
        msgpack::unpack(cursor, name, msg.streamId, msg.data.vdata());
        msg.name = static_cast<StreamName>(name);

        receive(msg);
    } else {
        UNREACHABLE;
    }
}

void WebRpcChannel::receive(const Msg& msg)
{
    RPCLOG() << "callId: " << msg.callId
             << ", method: " << to_string(msg.method)
             << ", type: " << to_string(msg.type)
             << ", data.size: " << msg.data.size();

    // all
    if (m_data.listenerAll) {
        m_data.listenerAll(msg);
    }

    // by method
    {
        auto it = m_data.onMethods.find(msg.method);
        if (it != m_data.onMethods.end() && it->second) {
            it->second(msg);
        }
    }

    // by callId (response)
    if (msg.type == MsgType::Response) {
        auto it = m_data.onResponses.find(msg.callId);
        if (it != m_data.onResponses.end() && it->second) {
            it->second(msg);
            m_data.onResponses.erase(it);
        }
    }
}

void WebRpcChannel::receive(const StreamMsg& msg)
{
    RPCLOG() << "received stream: " << to_string(msg.name)
             << ", streamId: " << msg.streamId
             << ", data.size: " << msg.data.size();

    auto it = m_data.onStreams.find(msg.streamId);
    if (it != m_data.onStreams.end() && it->second) {
        it->second(msg);
    }
}

void WebRpcChannel::onMethod(Method method, Handler h)
{
    m_data.onMethods[method] = h;
}

void WebRpcChannel::listenAll(Handler h)
{
    m_data.listenerAll = h;
}

void WebRpcChannel::addStream(std::shared_ptr<IRpcStream> s)
{
    s->init();

    m_data.streams.insert({ s->streamId(), s });
}

void WebRpcChannel::removeStream(StreamId id)
{
    auto removeStrm = [](std::map<StreamId, std::shared_ptr<IRpcStream> >& streams, StreamId id) {
        auto it = streams.find(id);
        if (it != streams.end()) {
            streams.erase(it);
        }
    };

    removeStrm(m_data.streams, id);
}

void WebRpcChannel::sendStream(const StreamMsg& msg)
{
    RPCLOG() << "stream: " << to_string(msg.name)
             << ", streamId: " << msg.streamId
             << ", data.size: " << msg.data.size();

    // clear but keep capacity
    buffer.clear();
    // makes sense if the reserve is greater than the current capacity
    buffer.reserve(std::max(msg.data.size(), DEFAULT_CAPACITY));

    msgpack::pack(buffer, STREAM_ID, (int)msg.name, msg.streamId, msg.data.constVData());

    IF_ASSERT_FAILED(buffer.size() > 0) {
        return;
    }

    rpcSend(&buffer[0], buffer.size());
}

void WebRpcChannel::onStream(StreamId id, StreamHandler h)
{
    m_data.onStreams[id] = h;
}
