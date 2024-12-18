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
#include "websocketapi.h"

#include <QWebSocket>

#include "log.h"

using namespace muse::network::api;

static int sSocketLastId = 1000;

WebSocketApi::WebSocketApi(muse::api::IApiEngine* e)
    : muse::api::ApiObject(e)
{}

WebSocketApi::~WebSocketApi()
{
    for (auto& p : m_sockets) {
        p.second.socket->close();
        p.second.socket->deleteLater();
    }
}

QUrl WebSocketApi::makeUrl(const QJSValue& conf) const
{
    QUrl url;
    url.setScheme("ws");
    url.setHost("localhost"); // default
    url.setPort(8084);        // default

    // just a port
    if (conf.isNumber()) {
        url.setPort(conf.toInt());
    }
    // url as string
    else if (conf.isString()) {
        url = QUrl(conf.toString());
    }
    //
    else if (conf.isObject()) {
        QJSValue u = conf.property("url");
        if (u.isString()) {
            url = QUrl(u.toString());
        } else {
            QJSValue port = conf.property("port");
            if (port.isNumber()) {
                url.setPort(port.toInt());
            }
            QJSValue host = conf.property("host");
            if (host.isString()) {
                url.setHost(host.toString());
            }
        }
    }

    return url;
}

void WebSocketApi::open(QJSValue conf, QJSValue onConnected)
{
    if (!onConnected.isCallable()) {
        LOGE() << "onConnected must be a function";
        return;
    }

    QUrl url = makeUrl(conf);
    if (!url.isValid()) {
        LOGE() << "Not valid url from conf: " << conf.toVariant();
        return;
    }

    int id = ++sSocketLastId;

    SocketData sd;
    sd.id = id;
    sd.socket = new QWebSocket();
    sd.onConnected = onConnected;

    m_sockets.insert({ sd.id, sd });

    QObject::connect(sd.socket, &QWebSocket::connected, this, [this, id]() {
        LOGD() << "socket connected id: " << id;
        auto it = m_sockets.find(id);
        IF_ASSERT_FAILED(it != m_sockets.end()) {
            return;
        }
        it->second.onConnected.call({ id });
    });

    QObject::connect(sd.socket, &QWebSocket::disconnected, this, [this, id]() {
        auto it = m_sockets.find(id);
        IF_ASSERT_FAILED(it != m_sockets.end()) {
            return;
        }

        LOGD() << "socket disconnected id: " << id << ", error: " << it->second.socket->errorString();

        it->second.socket->deleteLater();
        m_sockets.erase(id);
    });

    LOGD() << "try open socket id: " << id << ", url: " << url.toString();
    sd.socket->open(url);
}

void WebSocketApi::close(int id)
{
    auto it = m_sockets.find(id);
    IF_ASSERT_FAILED(it != m_sockets.end()) {
        return;
    }
    it->second.socket->close();
}

void WebSocketApi::onMessage(int id, QJSValue onMessage)
{
    if (!onMessage.isCallable()) {
        LOGE() << "onMessage must be a function";
        return;
    }

    auto it = m_sockets.find(id);
    IF_ASSERT_FAILED(it != m_sockets.end()) {
        return;
    }

    it->second.onMessage = onMessage;

    QObject::connect(it->second.socket, &QWebSocket::textMessageReceived, this, [this, id](const QString& message) {
        LOGD() << "messageReceived socket id: " << id << ", message: " << message;
        auto it = m_sockets.find(id);
        IF_ASSERT_FAILED(it != m_sockets.end()) {
            return;
        }
        it->second.onMessage.call({ message });
    });
}

void WebSocketApi::send(int id, const QString& msg)
{
    LOGD() << "send socket id: " << id << ", message: " << msg;
    auto it = m_sockets.find(id);
    IF_ASSERT_FAILED(it != m_sockets.end()) {
        return;
    }
    it->second.socket->sendTextMessage(msg);
}
