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
#include "ipcsocket.h"

#include <QLocalSocket>
#include <QUuid>

#include "async/async.h"
#include "ipclock.h"
#include "ipclog.h"

using namespace mu::ipc;

IpcSocket::~IpcSocket()
{
    delete m_socket;
    delete m_lock;
}

const ID& IpcSocket::selfID() const
{
    if (m_selfID.isEmpty()) {
        m_selfID = QUuid::createUuid().toString(QUuid::Id128);
    }
    return m_selfID;
}

bool IpcSocket::connect(const QString& serverName)
{
    if (!m_socket) {
        m_lock = new IpcLock(serverName);
        m_socket = new QLocalSocket();

        QObject::connect(m_socket, &QLocalSocket::errorOccurred, [this](QLocalSocket::LocalSocketError err) {
            //! NOTE If the server is down, then we will try to connect to another or create a server ourselves
            if (err == QLocalSocket::PeerClosedError) {
                m_disconected.notify();
            }
        });

        QObject::connect(m_socket, &QLocalSocket::readyRead, [this]() {
            onReadyRead();
        });
    }

    if (m_socket->state() == QLocalSocket::ConnectedState) {
        return true;
    }

    m_socket->connectToServer(serverName);
    bool ok = m_socket->waitForConnected(TIMEOUT_MSEC);
    if (!ok) {
        LOGW() << "failed connect to server";
        return false;
    }

    m_socket->write(selfID().toUtf8());

    ok = m_socket->waitForBytesWritten(TIMEOUT_MSEC);
    if (!ok) {
        LOGE() << "failed init socket";
    }

    LOGI() << "success connected to ipc server";

    return true;
}

mu::async::Notification IpcSocket::disconected()
{
    return m_disconected;
}

bool IpcSocket::send(const Msg& msg)
{
    IF_ASSERT_FAILED(m_socket) {
        return false;
    }

    Meta meta;
    meta.id = selfID();

    QByteArray data;
    serialize(meta, msg, data);

    IPCLOG() << data;

    m_lock->lock();

    m_socket->write(data);
    bool ok = m_socket->waitForBytesWritten(TIMEOUT_MSEC);

    m_lock->unlock();

    if (!ok) {
        LOGE() << "failed send to socket";
    }
    return ok;
}

void IpcSocket::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    IPCLOG() << data;
    async::Async::call(this, [this, data]() {
        onDataReceived(data);
    });
}

void IpcSocket::onDataReceived(const QByteArray& data)
{
    IPCLOG() << "received: " << data;

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

    m_msgReceived.send(receivedMsg);
}

void IpcSocket::onIpcMsg(const Meta& receivedMeta, const Msg& receivedMsg)
{
    IPCLOG() << "received ipc msg: " << receivedMsg.method;

    // answer on who is
    if (receivedMsg.method == IPC_WHOIS) {
        Msg answerMsg;
        answerMsg.destID = SERVER_ID;
        answerMsg.method = IPC_WHOIS;
        answerMsg.args << selfID();
        send(answerMsg);
        return;
    }

    // receive meta info
    if (receivedMsg.method == IPC_METAINFO) {
        IPCLOG() << "received meta info from: " << receivedMeta.id << ", args: " << receivedMsg.args;

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
            m_instances.append(meta);
        }

        m_instancesChanged.notify();
    }
}

mu::async::Channel<Msg> IpcSocket::msgReceived() const
{
    return m_msgReceived;
}

QList<Meta> IpcSocket::instances() const
{
    return m_instances;
}

mu::async::Notification IpcSocket::instancesChanged() const
{
    return m_instancesChanged;
}
