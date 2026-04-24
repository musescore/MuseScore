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

 #include "contextrpcchannelcontroller.h"

using namespace muse::audio::rpc;

void ContextRpcChannelController::send(const Msg& msg, const Handler& onResponse)
{
    globalChannel()->send(msg, onResponse);
}

void ContextRpcChannelController::onRequest(CtxId ctxId, MsgCode code, Handler h)
{
    if (h) {
        m_onRequests[code][ctxId] = h;
    } else {
        m_onRequests[code].erase(ctxId);
    }

    if (m_onRequests[code].empty()) {
        globalChannel()->onRequest(code, nullptr);
    } else {
        globalChannel()->onRequest(code, [this, code](const Msg& msg) {
            if (m_onRequests[code].find(msg.ctxId) != m_onRequests[code].end()) {
                m_onRequests[code][msg.ctxId](msg);
            }
        });
    }
}

void ContextRpcChannelController::onNotification(CtxId ctxId, MsgCode code, Handler h)
{
    if (h) {
        m_onNotifications[code][ctxId] = h;
    } else {
        m_onNotifications[code].erase(ctxId);
    }

    if (m_onNotifications[code].empty()) {
        globalChannel()->onNotification(code, nullptr);
    } else {
        globalChannel()->onNotification(code, [this, code](const Msg& msg) {
            if (m_onNotifications[code].find(msg.ctxId) != m_onNotifications[code].end()) {
                m_onNotifications[code][msg.ctxId](msg);
            }
        });
    }
}

void ContextRpcChannelController::addStream(std::shared_ptr<IRpcStream> s)
{
    globalChannel()->addStream(s);
}

void ContextRpcChannelController::removeStream(StreamId id)
{
    globalChannel()->removeStream(id);
}

void ContextRpcChannelController::sendStream(const StreamMsg& msg)
{
    globalChannel()->sendStream(msg);
}

void ContextRpcChannelController::onStream(StreamId id, StreamHandler h)
{
    globalChannel()->onStream(id, h);
}
