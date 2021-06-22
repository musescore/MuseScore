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

#include "ipclog.h"

void mu::ipc::serialize(const Msg& msg, QByteArray& data)
{
    QJsonObject msgObj;

    msgObj["srcID"] = msg.srcID;
    msgObj["destID"] = msg.destID;
    msgObj["type"] = static_cast<int>(msg.type);
    msgObj["method"] = msg.method;

    QJsonArray argsArr;
    for (const QString& arg : qAsConst(msg.args)) {
        argsArr.append(arg);
    }
    msgObj["args"] = argsArr;

    data = QJsonDocument(msgObj).toJson(QJsonDocument::Compact);
}

void mu::ipc::deserialize(const QByteArray& data, Msg& msg)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject msgObj = doc.object();

    msg.srcID =  msgObj.value("srcID").toString();
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

bool mu::ipc::writeToSocket(QLocalSocket* socket, const QByteArray& data)
{
    QDataStream stream(socket);
    stream.writeBytes(data.constData(), data.size());
    bool ok = socket->waitForBytesWritten(ipc::TIMEOUT_MSEC);
    if (!ok) {
        LOGE() << "failed write to socket, err: " << socket->errorString();
    }
    return ok;
}

bool mu::ipc::readFromSocket(QLocalSocket* socket, std::function<void(const QByteArray& data)> onPackegReaded)
{
    qint64 bytesAvailable = socket->bytesAvailable();
    if (bytesAvailable < (qint64)sizeof(quint32)) {
        return false;
    }

    int packageCount = 0;
    QDataStream stream(socket);

    auto readPackage = [socket, &stream, onPackegReaded]() {
        QByteArray data;
        quint32 remaining;
        stream >> remaining;
        data.resize(remaining);

        qint64 available = socket->bytesAvailable();
        if (available < remaining) {
            if (!socket->waitForReadyRead(ipc::TIMEOUT_MSEC)) {
                LOGE() << "failed read, remaining: " << remaining << ", available: " << available << ", err: " << socket->errorString();
                return false;
            }
        }

        char* ptr = data.data();
        int readed = stream.readRawData(ptr, remaining);
        if (quint32(readed) != remaining) {
            LOGE() << "failed read from socket";
            return false;
        }

        onPackegReaded(data);
        return true;
    };

    bool ok = true;
    while (socket->bytesAvailable() > 0) {
        ok = readPackage();
        ++packageCount;
        if (!ok) {
            break;
        }
    }

    IPCLOG() << "readed package count: " << packageCount;

    if (!ok) {
        LOGE() << "failed read package";
    }
    return ok;
}
