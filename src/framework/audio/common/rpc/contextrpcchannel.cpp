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

#include "contextrpcchannel.h"

using namespace muse::audio::rpc;

ContextRpcChannel::~ContextRpcChannel()
{
    for (auto id : m_streams) {
        m_controller->removeStream(id);
    }
}

void ContextRpcChannel::send(const Msg& msg, const Handler& onResponse)
{
    Msg newMsg = msg;
    newMsg.ctxId = contextId();
    m_controller->send(newMsg, onResponse);
}

void ContextRpcChannel::onRequest(MsgCode code, Handler h)
{
    m_controller->onRequest(contextId(), code, h);
}

void ContextRpcChannel::onNotification(MsgCode code, Handler h)
{
    m_controller->onNotification(contextId(), code, h);
}

void ContextRpcChannel::addStream(std::shared_ptr<IRpcStream> s)
{
    s->setCtxId(contextId());
    m_streams.insert(s->streamId());
    m_controller->addStream(s);
}

void ContextRpcChannel::removeStream(StreamId id)
{
    m_streams.erase(id);
    m_controller->removeStream(id);
}

void ContextRpcChannel::sendStream(const StreamMsg& msg)
{
    //! NOTE The context ID is already set in the stream.
    DO_ASSERT(msg.ctxId == contextId());
    m_controller->sendStream(msg);
}

void ContextRpcChannel::onStream(StreamId id, StreamHandler h)
{
    m_controller->onStream(id, h);
}
