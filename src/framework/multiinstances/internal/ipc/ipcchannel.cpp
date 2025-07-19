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

#include <QTimer>

#include "ipcsocket.h"
#include "ipcserver.h"
#include "ipclock.h"
#include "ipcloop.h"

#include "log.h"

namespace muse::ipc {
IpcChannel::IpcChannel()
{
    m_selfSocket = new IpcSocket();
    m_selfSocket->disconnected().onNotify(this, [this]() { onDisconnected(); });
    m_selfSocket->msgReceived().onReceive(this, [this](const Msg& msg) { onSocketMsgReceived(msg); });
}

IpcChannel::~IpcChannel()
{
    delete m_selfSocket;
    delete m_server;
}

const ID& IpcChannel::selfID() const
{
    return m_selfSocket->selfID();
}

bool IpcChannel::isServer() const
{
    return m_server != nullptr;
}

void IpcChannel::connect()
{
    setupConnection();
}

bool IpcChannel::send(const Msg& msg)
{
    setupConnection();
    return m_selfSocket->send(msg);
}

void IpcChannel::response(const QString& method, const QStringList& args, const ID& destID)
{
    Msg res;
    res.destID = destID;
    res.type = MsgType::Response;
    res.method = method;
    res.args = args;
    send(res);
}

void IpcChannel::broadcast(const QString& method, const QStringList& args)
{
    Msg res;
    res.destID = BROADCAST_ID;
    res.type = MsgType::Notify;
    res.method = method;
    res.args = args;
    send(res);
}

Code IpcChannel::syncRequestToAll(const QString& method, const QStringList& args, const OnReceived& onReceived)
{
    IF_ASSERT_FAILED(onReceived) {
        return Code::Undefined;
    }

    Msg msg;
    msg.destID = ipc::BROADCAST_ID;
    msg.type = MsgType::Request;
    msg.method = method;
    msg.args = args;

    IpcLoop loop;

    int total = m_selfSocket->instances().count();
    total -= 1;     //! NOTE Exclude itself
    if (total <= 0) {
        return Code::AllAnswered;
    }
    int received = 0;

    m_msgCallback = [method, total, &received, &loop, onReceived](const Msg& msg) {
        if (!(msg.type == MsgType::Response && msg.method == method)) {
            return;
        }

        ++received;
        bool success = onReceived(msg.args, msg.srcID);
        if (success) {
            loop.exit(Code::Success);
            return;
        }

        if (received == total) {
            loop.exit(Code::AllAnswered);
        }
    };

    send(msg);

    Code code = loop.exec(500);
    LOGD() << "ret code: " << int(code);

    m_msgCallback = nullptr;

    return code;
}

void IpcChannel::onSocketMsgReceived(const Msg& msg)
{
    if (m_msgCallback) {
        m_msgCallback(msg);
    }

    m_msgReceived.send(msg);
}

async::Channel<Msg> IpcChannel::msgReceived() const
{
    return m_msgReceived;
}

void IpcChannel::setupConnection()
{
    if (!m_selfSocket->connect(SERVER_NAME)) {
        IpcLock lock(SERVER_NAME);
        lock.lock();

        //! NOTE Check again
        if (!m_selfSocket->connect(SERVER_NAME)) {
            //! NOTE If it was not possible to connect to the server, then it no there or was, but it was closed.
            //! In this case, we will become a server
            m_server = new IpcServer();
            m_server->listen(SERVER_NAME);
        }
        lock.unlock();

        //! NOTE Connect to self server
        m_selfSocket->connect(SERVER_NAME);
    }
}

void IpcChannel::onDisconnected()
{
    //! NOTE If the server is down, then we will try to connect to another or create a server ourselves
    uint64_t min = 1;
    uint64_t max = 100;
    //! NOTE All sockets receive a disconnect notify at the same time, add some delay to reduce the likelihood of simultaneous server creation
    int interval = static_cast<int>(reinterpret_cast<uint64_t>(this) % (max - min + 1) + min);
    QTimer::singleShot(interval, [this]() {
        setupConnection();
    });
}

QList<ID> IpcChannel::instances() const
{
    return m_selfSocket->instances();
}

async::Notification IpcChannel::instancesChanged() const
{
    return m_selfSocket->instancesChanged();
}
}
