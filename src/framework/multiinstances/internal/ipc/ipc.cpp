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

#include <QDataStream>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QLocalSocket>

#include "ipclog.h"

void muse::ipc::serialize(const Msg& msg, QByteArray& data)
{
    QJsonObject msgObj;

    msgObj["srcID"] = msg.srcID;
    msgObj["destID"] = msg.destID;
    msgObj["type"] = static_cast<int>(msg.type);
    msgObj["method"] = msg.method;

    QJsonArray argsArr;
    for (const QString& arg : std::as_const(msg.args)) {
        argsArr.append(arg);
    }
    msgObj["args"] = argsArr;

    data = QJsonDocument(msgObj).toJson(QJsonDocument::Compact);
}

void muse::ipc::deserialize(const QByteArray& data, Msg& msg)
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

QString muse::ipc::socketErrorToString(int err)
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

bool muse::ipc::writeToSocket(QLocalSocket* socket, const QByteArray& data)
{
    QDataStream stream(socket);
    stream.writeBytes(data.constData(), data.size());
    bool ok = socket->waitForBytesWritten(ipc::TIMEOUT_MSEC);
    if (!ok) {
        LOGE() << "failed write to socket, err: " << socket->errorString();
    }
    return ok;
}

bool muse::ipc::readFromSocket(QLocalSocket* socket, std::function<void(const QByteArray& data)> onPackageRead)
{
    qint64 bytesAvailable = socket->bytesAvailable();
    if (bytesAvailable < (qint64)sizeof(quint32)) {
        return false;
    }

    int packageCount = 0;
    QDataStream stream(socket);

    auto readPackage = [socket, &stream, onPackageRead]() {
        static constexpr uint32_t MAX_PACKAGE_SIZE = 2048;

        QByteArray data;
        uint32_t remaining;
        stream >> remaining;
        IF_ASSERT_FAILED(remaining <= MAX_PACKAGE_SIZE) {
            return false;
        }
        data.resize(remaining);

        int64_t available = socket->bytesAvailable();
        if (available < remaining) {
            if (!socket->waitForReadyRead(ipc::TIMEOUT_MSEC)) {
                LOGE() << "failed read, remaining: " << remaining << ", available: " << available << ", err: " << socket->errorString();
                return false;
            }
        }

        char* ptr = data.data();
        int read = stream.readRawData(ptr, remaining);
        if (quint32(read) != remaining) {
            LOGE() << "failed read from socket";
            return false;
        }

        onPackageRead(data);
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

    IPCLOG() << "read package count: " << packageCount;

    if (!ok) {
        LOGE() << "failed read package";
    }
    return ok;
}
