/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "ipc.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QLocalSocket>

void mu::ipc::serialize(const Meta& meta, const Msg& msg, QByteArray& data)
{
    QJsonObject obj;

    QJsonObject metaObj;
    metaObj["id"] = meta.id;

    obj["meta"] = metaObj;

    QJsonObject msgObj;
    msgObj["destID"] = msg.destID;
    msgObj["type"] = static_cast<int>(msg.type);
    msgObj["method"] = msg.method;

    QJsonArray argsArr;
    for (const QString& arg : qAsConst(msg.args)) {
        argsArr.append(arg);
    }
    msgObj["args"] = argsArr;

    obj["msg"] = msgObj;

    data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

void mu::ipc::deserialize(const QByteArray& data, Meta& meta, Msg& msg)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    QJsonObject metaObj = obj.value("meta").toObject();
    meta.id = metaObj.value("id").toString();

    QJsonObject msgObj = obj.value("msg").toObject();
    msg.destID =  msgObj.value("destID").toString();
    msg.type = static_cast<MsgType>(msgObj.value("type").toInt());
    msg.method = msgObj.value("method").toString();

    QJsonArray argsArr = msgObj.value("args").toArray();
    for (int i = 0; i < argsArr.count(); ++i) {
        msg.args << argsArr.at(i).toString();
    }
}

QString mu::ipc::socketErrorToString(int err)
{
    switch (err) {
    case QLocalSocket::ConnectionRefusedError: return "ConnectionRefusedError";
    case QLocalSocket::PeerClosedError: return "PeerClosedError";
    case QLocalSocket::ServerNotFoundError: return "ServerNotFoundError";
    case QLocalSocket::SocketAccessError: return "SocketAccessError";
    case QLocalSocket::SocketResourceError: return "SocketResourceError";
    case QLocalSocket::SocketTimeoutError: return "SocketTimeoutError";
    case QLocalSocket::DatagramTooLargeError: return "DatagramTooLargeError";
    case QLocalSocket::ConnectionError: return "ConnectionError";
    case QLocalSocket::UnsupportedSocketOperationError: return "UnsupportedSocketOperationError";
    case QLocalSocket::UnknownSocketError: return "UnknownSocketError";
    case QLocalSocket::OperationError: return "OperationError";
    }
    return "Unknown error";
}
