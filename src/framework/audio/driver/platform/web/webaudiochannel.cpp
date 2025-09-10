/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "webaudiochannel.h"

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include "log.h"

using namespace muse::audio::worker;

static std::function<void(const emscripten::val& msg)> g_rpcListen = nullptr;

static const std::string NOTIFY_INITED_TYPE = "channel_inited";
static const std::string REQUEST_AUDIO_TYPE = "request_audio";
static const std::string RESPONSE_AUDIO_TYPE = "response_audio";

static void rpcSend(const std::string& type, const std::vector<float>& data)
{
    emscripten::val msg = emscripten::val::object();
    msg.set("type", emscripten::val(type));
    if (!data.empty()) {
        emscripten::val jsArray = emscripten::val::global("Float32Array").new_(
            emscripten::typed_memory_view(data.size(), &data[0])
            );

        msg.set("data", jsArray);
    }

    emscripten::val::module_property("driver_worker_rpcSend")(msg);
}

static void rpcListen(const emscripten::val& msg)
{
    if (g_rpcListen) {
        g_rpcListen(msg);
    }
}

EMSCRIPTEN_BINDINGS(RpcChannel) {
    function("driver_worker_rpcListen", &rpcListen);
}

void WebAudioChannel::open(Callback callback)
{
    IF_ASSERT_FAILED(callback) {
        return;
    }

    m_callback = callback;

    g_rpcListen = [this](const emscripten::val& msg) {
        std::string type = msg["type"].as<std::string>();
        // LOGDA() << "recived msg type: " << type;
        if (type == REQUEST_AUDIO_TYPE) {
            int samplesPerChannel = msg["samplesPerChannel"].as<int>();

            IF_ASSERT_FAILED(samplesPerChannel >= 128) {
                return;
            }

            IF_ASSERT_FAILED(m_callback) {
                return;
            }

            // clear but keep capacity
            m_buffer.clear();
            m_buffer.resize(samplesPerChannel * 2, 0.f);

            m_callback(&m_buffer[0], m_buffer.size());

            rpcSend(RESPONSE_AUDIO_TYPE, m_buffer);
        }
    };

    rpcSend(NOTIFY_INITED_TYPE, {});
}

void WebAudioChannel::close()
{
    m_callback = nullptr;
    g_rpcListen = nullptr;
}
