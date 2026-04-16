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

void ContextRpcChannel::send(const Msg& msg, const Handler& onResponse)
{
    Msg m = msg;
    m.ctxId = contextId();
    globalChannel()->send(m, [this, onResponse](const Msg& msg) {
        DO_ASSERT(msg.ctxId == contextId());
        onResponse(msg);
    });
}

void ContextRpcChannel::onMethod(Method method, Handler h)
{
    globalChannel()->onMethod(method, [this, h](const Msg& msg) {
        if (msg.ctxId == contextId()) {
            h(msg);
        } else if (msg.type == MsgType::Notification) { // Notifications are not contextual yet
            h(msg);
        }
    });
}

void ContextRpcChannel::addStream(std::shared_ptr<IRpcStream> s)
{
    s->setCtxId(contextId());
    globalChannel()->addStream(s);
}

void ContextRpcChannel::removeStream(StreamId id)
{
    globalChannel()->removeStream(id);
}

void ContextRpcChannel::sendStream(const StreamMsg& msg)
{
    //! NOTE The context ID is already set in the stream.
    DO_ASSERT(msg.ctxId == contextId());
    globalChannel()->sendStream(msg);
}

void ContextRpcChannel::onStream(StreamId id, StreamHandler h)
{
    globalChannel()->onStream(id, h);
}
