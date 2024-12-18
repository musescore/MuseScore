/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#include "websocketserverapi.h"

#include <QWebSocketServer>
#include <QWebSocket>

#include "log.h"

using namespace muse::network::api;

static int sClientLastId = 2000;

WebSocketServerApi::WebSocketServerApi(muse::api::IApiEngine* e)
    : muse::api::ApiObject(e)
{
    m_server = new QWebSocketServer("muse_extension", QWebSocketServer::NonSecureMode);

    QObject::connect(m_server, &QWebSocketServer::newConnection, this, &WebSocketServerApi::onNewConnection);
}

WebSocketServerApi::~WebSocketServerApi()
{
    delete m_server;
}

void WebSocketServerApi::listen(int port, QJSValue onNewConnected)
{
    if (port < 0) {
        port = 8084;
    }

    if (!onNewConnected.isCallable()) {
        LOGE() << "onNewConnected must be a function";
        return;
    }

    m_onNewConnected = onNewConnected;

    LOGD() << "server listen port: " << port;
    m_server->listen(QHostAddress::Any, port);
}

void WebSocketServerApi::onNewConnection()
{
    int id = ++sClientLastId;

    LOGD() << "onNewConnection client id: " << id;

    SocketData sd;
    sd.id = id;
    sd.socket = m_server->nextPendingConnection();

    m_clients.insert({ sd.id, sd });

    QObject::connect(sd.socket, &QWebSocket::disconnected, this, [this, id]() {
        auto it = m_clients.find(id);
        IF_ASSERT_FAILED(it != m_clients.end()) {
            return;
        }

        LOGD() << "disconnected client id: " << id << ", error: " << it->second.socket->errorString();

        it->second.socket->deleteLater();
        m_clients.erase(id);
    });

    m_onNewConnected.call({ id });
}

void WebSocketServerApi::onMessage(int id, QJSValue onMessage)
{
    if (!onMessage.isCallable()) {
        LOGE() << "onMessage must be a function";
        return;
    }

    auto it = m_clients.find(id);
    IF_ASSERT_FAILED(it != m_clients.end()) {
        return;
    }

    it->second.onMessage = onMessage;

    QObject::connect(it->second.socket, &QWebSocket::textMessageReceived, this, [this, id](const QString& message) {
        LOGD() << "messageReceived client id: " << id << ", message: " << message;
        auto it = m_clients.find(id);
        IF_ASSERT_FAILED(it != m_clients.end()) {
            return;
        }
        it->second.onMessage.call({ message });
    });
}

void WebSocketServerApi::send(int id, const QString& msg)
{
    LOGD() << "send client id: " << id << ", message: " << msg;
    auto it = m_clients.find(id);
    IF_ASSERT_FAILED(it != m_clients.end()) {
        return;
    }
    it->second.socket->sendTextMessage(msg);
}
