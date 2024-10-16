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
#pragma once

#include <map>
#include <QJSValue>
#include <QUrl>

#include "global/api/apiobject.h"

class QWebSocket;
namespace muse::network::api {
class WebSocketApi : public muse::api::ApiObject
{
    Q_OBJECT
public:
    WebSocketApi(muse::api::IApiEngine* e);
    ~WebSocketApi();

    Q_INVOKABLE void open(QJSValue conf, QJSValue onConnected /* void(int id)*/);
    Q_INVOKABLE void close(int id);
    Q_INVOKABLE void onMessage(int id, QJSValue onMessage);
    Q_INVOKABLE void send(int id, const QString& msg);

private:

    QUrl makeUrl(const QJSValue& conf) const;

    struct SocketData {
        int id = -1;
        QWebSocket* socket = nullptr;
        QJSValue onConnected;
        QJSValue onMessage;
    };

    std::map<int /*id*/, SocketData> m_sockets;
};
}
