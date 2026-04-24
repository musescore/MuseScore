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
    RPCLOG() << "ctxId: " << msg.ctxId
             << ", callId: " << msg.callId
             << ", code: " << to_string(msg.code)
             << ", type: " << to_string(msg.type)
             << ", data.size: " << msg.data.size();

    // clear but keep capacity
    buffer.clear();
    // makes sense if the reserve is greater than the current capacity
    buffer.reserve(std::max(msg.data.size(), DEFAULT_CAPACITY));

    msgpack::pack(buffer, msg.ctxId, msg.callId, (uint8_t)msg.code, (uint8_t)msg.type, msg.data.constVData());

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

    Msg msg;
    uint8_t code = 0;
    uint8_t type = 0;
    msgpack::unpack(cursor, msg.ctxId, msg.callId, code, type, msg.data.vdata());
    msg.code = static_cast<MsgCode>(code);
    msg.type = static_cast<MsgType>(type);

    receive(msg);
}

void WebRpcChannel::receive(const Msg& msg)
{
    RPCLOG() << "ctxId: " << msg.ctxId
             << ", callId: " << msg.callId
             << ", code: " << to_string(msg.code)
             << ", type: " << to_string(msg.type)
             << ", data.size: " << msg.data.size();

    // all
    if (m_data.listenerAll) {
        m_data.listenerAll(msg);
    }
    switch (msg.type) {
    case MsgType::Stream: {
        auto it = m_data.onStreams.find(msg.callId);
        if (it != m_data.onStreams.end() && it->second) {
            it->second(msg);
        }
    } break;
    case MsgType::Request: {
        auto it = m_data.onRequests.find(msg.code);
        if (it != m_data.onRequests.end() && it->second) {
            it->second(msg);
        }
    } break;
    case MsgType::Notification: {
        auto it = m_data.onNotifications.find(msg.code);
        if (it != m_data.onNotifications.end() && it->second) {
            it->second(msg);
        }
    } break;
    case MsgType::Response: {
        auto it = m_data.onResponses.find(msg.callId);
        if (it != m_data.onResponses.end()) {
            if (it->second) {
                it->second(msg);
            }
            m_data.onResponses.erase(it);
        }
    } break;
    default: {
        UNREACHABLE;
        break;
    }
    }
}

void WebRpcChannel::onRequest(MsgCode code, Handler h)
{
    m_data.onRequests[code] = h;
}

void WebRpcChannel::onNotification(MsgCode code, Handler h)
{
    m_data.onNotifications[code] = h;
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
    send(msg);
}

void WebRpcChannel::onStream(StreamId id, StreamHandler h)
{
    m_data.onStreams[id] = h;
}
