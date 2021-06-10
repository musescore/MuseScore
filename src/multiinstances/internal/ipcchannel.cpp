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
#include "ipcchannel.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QSystemSemaphore>
#include <QUuid>
#include <QDataStream>
#include <QTimer>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#ifdef Q_OS_UNIX
#include <QFile>
#include <QDir>
#endif

#include "log.h"

//#define IPC_LOGGING_ENABLED

#undef MYLOG
#ifdef IPC_LOGGING_ENABLED
#define MYLOG() LOGI()
#else
#define MYLOG() LOGN()
#endif

using namespace mu::mi;

static const QString SERVER_NAME("musescore-app-ipc");
static const int RW_TIMEOUT = 500; // ms

static const int CHECK_CONNECT_INTERVAL = 500; // ms

static const QString DEST_BROADCAST("broadcast");
static const QString DEST_SOCKET("socket"); // direct to socket
static const QString DEST_SERVER("server"); // direct to server

static const QString IPC_("ipc_");
static const QString IPC_INIT("ipc_init");
static const QString IPC_WHOIS("ipc_whois");
static const QString IPC_METAINFO("ipc_metainfo");
static const QString IPC_PING("ipc_ping");

static QString socketErrorToString(QLocalSocket::LocalSocketError err)
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

IpcChannel::~IpcChannel()
{
    delete m_serverLock;
    delete m_selfSocket;
    delete m_server;
}

// ============================================================
// Common
// ============================================================

const QString& IpcChannel::selfID() const
{
    if (!m_selfID.isEmpty()) {
        return m_selfID;
    }

    m_selfID = QUuid::createUuid().toString(QUuid::Id128);
    return m_selfID;
}

bool IpcChannel::doSendToSocket(QLocalSocket* socket, const QByteArray& data) const
{
    QDataStream stream(socket);
    stream.writeBytes(data.constData(), data.size());
    bool ok = socket->waitForBytesWritten(RW_TIMEOUT);
    if (!ok) {
        LOGE() << "failed send to socket";
    }
    return ok;
}

void IpcChannel::doReadSocket(QLocalSocket* socket, QByteArray& data) const
{
    QDataStream stream(socket);
    quint32 remaining;
    stream >> remaining;
    data.resize(remaining);

    char* ptr = data.data();
    stream.readRawData(ptr, remaining);
}

void IpcChannel::serialize(const Meta& meta, const Msg& msg, QByteArray& data) const
{
    QJsonObject obj;

    QJsonObject metaObj;
    metaObj["id"] = meta.id;
    metaObj["isServer"] = meta.isServer;

    obj["meta"] = metaObj;

    QJsonObject msgObj;
    msgObj["destID"] = msg.destID;
    msgObj["requestID"] = msg.requestID;
    msgObj["method"] = msg.method;

    QJsonArray argsArr;
    for (const QString& arg : qAsConst(msg.args)) {
        argsArr.append(arg);
    }
    msgObj["args"] = argsArr;

    obj["msg"] = msgObj;

    data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

void IpcChannel::deserialize(const QByteArray& data, Meta& meta, Msg& msg) const
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    QJsonObject metaObj = obj.value("meta").toObject();
    meta.id = metaObj.value("id").toString();
    meta.isServer = metaObj.value("isServer").toBool();

    QJsonObject msgObj = obj.value("msg").toObject();
    msg.destID =  msgObj.value("destID").toString();
    msg.requestID = msgObj.value("requestID").toInt();
    msg.method = msgObj.value("method").toString();

    QJsonArray argsArr = msgObj.value("args").toArray();
    for (int i = 0; i < argsArr.count(); ++i) {
        msg.args << argsArr.at(i).toString();
    }
}

// ============================================================
// Self socket
// ============================================================

bool IpcChannel::init()
{
    Msg msg;
    msg.destID = DEST_SERVER;
    msg.method = IPC_INIT;
    return send(msg);
}

void IpcChannel::ping()
{
    Msg msg;
    msg.destID = DEST_SERVER;
    msg.method = IPC_PING;
    send(msg);
}

bool IpcChannel::send(const Msg& msg)
{
    if (!checkConnectToServer()) {
        if (!m_serverLock) {
            m_serverLock = new QSystemSemaphore(SERVER_NAME, 1, QSystemSemaphore::Open);
        }

        m_serverLock->acquire();
        //! NOTE Check again
        if (!checkConnectToServer()) {
            //! NOTE If it was not possible to connect to the server, then it no there or was, but it was closed.
            //! In this case, we will become a server
            makeServer();
        }
        m_serverLock->release();

        //! NOTE Connect to self server
        checkConnectToServer();
    }

    return sendToSocket(m_selfSocket, msg);
}

bool IpcChannel::sendToSocket(QLocalSocket* socket, const Msg& msg) const
{
    Meta meta;
    meta.id = selfID();
    meta.isServer = selfIsServer();

    QByteArray data;
    serialize(meta, msg, data);
    return doSendToSocket(socket, data);
}

void IpcChannel::onDataReceived(const QByteArray& data)
{
    MYLOG() << "received: " << data;

    Meta receivedMeta;
    Msg receivedMsg;
    deserialize(data, receivedMeta, receivedMsg);

    if (receivedMsg.method.startsWith(IPC_)) {
        onIpcMsg(receivedMeta, receivedMsg);
        return;
    }

    if (receivedMeta.id == selfID()) {
        return;
    }

    m_listenCh.send(receivedMsg);
}

void IpcChannel::onIpcMsg(const Meta& receivedMeta, const Msg& receivedMsg)
{
    MYLOG() << "received ipc msg: " << receivedMsg.method;

    // answer on who is
    if (receivedMsg.method == IPC_WHOIS) {
        Msg answerMsg;
        answerMsg.destID = DEST_SERVER;
        answerMsg.method = IPC_WHOIS;
        answerMsg.args << selfID();
        sendToSocket(m_selfSocket, answerMsg);
        return;
    }

    // receive meta info
    if (receivedMsg.method == IPC_METAINFO) {
        MYLOG() << "received meta info from: " << receivedMeta.id << ", args: " << receivedMsg.args;

        IF_ASSERT_FAILED(!receivedMsg.args.isEmpty()) {
            return;
        }

        int count = receivedMsg.args.at(0).toInt();
        IF_ASSERT_FAILED(count > 0) {
            return;
        }

        m_instances.clear();
        count += 1;
        for (int i = 1; i < count; ++i) {
            Meta meta;
            meta.id = receivedMsg.args.at(i);
            meta.isServer = receivedMeta.id == meta.id; // must be received only from server
            m_instances.append(meta);
        }

        m_instancesChanged.notify();
    }
}

mu::async::Channel<IpcChannel::Msg> IpcChannel::listen()
{
    return m_listenCh;
}

bool IpcChannel::checkConnectToServer()
{
    if (!m_selfSocket) {
        m_selfSocket = new QLocalSocket();

        QObject::connect(m_selfSocket, &QLocalSocket::errorOccurred, [this](QLocalSocket::LocalSocketError err) {
            MYLOG() << "socket error: " << socketErrorToString(err);

            //! NOTE If the server is down, then we will try to connect to another or create a server ourselves
            if (err == QLocalSocket::PeerClosedError) {
                uint64_t min = 1;
                uint64_t max = 100;
                int interval = static_cast<int>(reinterpret_cast<uint64_t>(this) % (max - min + 1) + min);
                QTimer::singleShot(interval, [this]() {
                    LOGI() << "try send ping";
                    ping();
                });
            }
        });

        QObject::connect(m_selfSocket, &QLocalSocket::readyRead, [this]() {
            QByteArray data;
            doReadSocket(m_selfSocket, data);
            onDataReceived(data);
        });
    }

    if (m_selfSocket->state() == QLocalSocket::ConnectedState) {
        return true;
    }

    m_selfSocket->connectToServer(SERVER_NAME);
    bool ok = m_selfSocket->waitForConnected(500);
    if (!ok) {
        LOGW() << "failed connect to server, err: " << socketErrorToString(m_selfSocket->error());
        return false;
    }

    LOGI() << "success connected to ipc server";

    return true;
}

QList<IpcChannel::Meta> IpcChannel::instances() const
{
    return m_instances;
}

mu::async::Notification IpcChannel::instancesChanged() const
{
    return m_instancesChanged;
}

// ============================================================
// Me as server
// ============================================================
bool IpcChannel::selfIsServer() const
{
    return m_server != nullptr;
}

bool IpcChannel::makeServer()
{
    LOGI() << "make new ipc server, selfID: " << selfID();
    m_server = new QLocalServer();

    bool ok = m_server->listen(SERVER_NAME);

#ifdef Q_OS_UNIX
    // Workaround
    if (!ok && m_server->serverError() == QAbstractSocket::AddressInUseError) {
        QFile::remove(QDir::cleanPath(QDir::tempPath()) + QLatin1Char('/') + SERVER_NAME);
        ok = m_server->listen(SERVER_NAME);
    }
#endif

    if (!ok) {
        LOGE() << "failed listen: " << m_server->errorString();
        return false;
    }

    QObject::connect(m_server, &QLocalServer::newConnection, [this]() {
        QLocalSocket* socket = m_server->nextPendingConnection();
        MYLOG() << "newConnection socket: " << socket;
        if (!socket) {
            return;
        }

        QObject::connect(socket, &QLocalSocket::errorOccurred, [](QLocalSocket::LocalSocketError err) {
            LOGE() << "socket error: " << socketErrorToString(err);
        });

        QObject::connect(socket, &QLocalSocket::disconnected, [socket, this]() {
            MYLOG() << "disconnected socket: " << socket;
            onDisconnected(socket);
        });

        QObject::connect(socket, &QLocalSocket::readyRead, [socket, this]() {
            onIncomingReadyRead(socket);
        });

        IncomingSocket inc;
        inc.socket = socket;
        m_incomingSockets.append(inc);

        askWhoIs(socket);
    });

    return true;
}

void IpcChannel::askWhoIs(QLocalSocket* socket) const
{
    Msg askMsg;
    askMsg.destID = DEST_SOCKET;
    askMsg.method = IPC_WHOIS;
    sendToSocket(socket, askMsg);
}

void IpcChannel::onIncomingReadyRead(QLocalSocket* socket)
{
    QByteArray data;
    doReadSocket(socket, data);

    Meta meta;
    Msg msg;
    deserialize(data, meta, msg);

    MYLOG() << "incoming [" << meta.id << "] data: " << data;

    if (msg.method == IPC_INIT) {
        onIncomingInit(socket, meta, msg);
    }

    if (msg.method == IPC_WHOIS) {
        onIncomingWhoIs(socket, meta, msg);
    }

    if (msg.method == IPC_PING) {
        onIncomingPing(socket, meta, msg);
    }

    //! NOTE Resend to others (broadcast)
    if (msg.destID == DEST_BROADCAST) {
        for (IncomingSocket& s : m_incomingSockets) {
            //! NOTE We do not resend to incoming socket
            //! In addition to the metainfo message, we send it to ourselves socket too
            if (socket != s.socket || msg.method == IPC_METAINFO) {
                MYLOG() << "resend to " << s.meta.id;
                doSendToSocket(s.socket, data);
            }
        }
    }
}

void IpcChannel::onIncomingInit(QLocalSocket* socket, const Meta& meta, const Msg& msg)
{
    UNUSED(msg);

    MYLOG() << "init from: " << meta.id;

    IncomingSocket& s = incomingSocket(socket);
    if (!s.socket) {
        LOGE() << "not found incoming socket";
        return;
    }
    s.meta = meta;

    sendMetaInfoToAllIncoming();
}

void IpcChannel::onIncomingWhoIs(QLocalSocket* socket, const Meta& meta, const Msg& msg)
{
    UNUSED(msg);

    MYLOG() << "who is answer: " << meta.id;

    IncomingSocket& s = incomingSocket(socket);
    if (!s.socket) {
        LOGE() << "not found incoming socket";
        return;
    }
    s.meta = meta;

    sendMetaInfoToAllIncoming();
}

void IpcChannel::onIncomingPing(QLocalSocket* socket, const Meta& meta, const Msg& msg)
{
    UNUSED(msg);

    MYLOG() << "ping from: " << meta.id;

    IncomingSocket& s = incomingSocket(socket);
    if (!s.socket) {
        LOGE() << "not found incoming socket";
        return;
    }
    s.meta = meta;

    sendMetaInfoToAllIncoming();
}

void IpcChannel::onDisconnected(QLocalSocket* socket)
{
    int index = -1;
    for (int i = 0; i < m_incomingSockets.count(); ++i) {
        if (m_incomingSockets.at(i).socket == socket) {
            index = i;
            break;
        }
    }

    if (index < 0) {
        LOGW() << "not found socket";
        return;
    }

    m_incomingSockets.removeAt(index);

    sendMetaInfoToAllIncoming();
}

IpcChannel::IncomingSocket& IpcChannel::incomingSocket(QLocalSocket* socket)
{
    for (IncomingSocket& s : m_incomingSockets) {
        if (s.socket == socket) {
            return s;
        }
    }

    static IncomingSocket null;
    return null;
}

void IpcChannel::sendMetaInfoToAllIncoming()
{
    Msg msg;
    msg.destID = DEST_BROADCAST;
    msg.method = IPC_METAINFO;

    msg.args << QString::number(m_incomingSockets.count());
    for (const IncomingSocket& s : qAsConst(m_incomingSockets)) {
        msg.args << s.meta.id;
    }

    sendToSocket(m_selfSocket, msg);
}
