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

using namespace mu::ipc;

IpcChannel::IpcChannel()
{
    m_selfSocket = new IpcSocket();
    m_selfSocket->disconected().onNotify(this, [this]() { onDisconected(); });
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

void IpcChannel::connect()
{
    setupConnection();
}

bool IpcChannel::send(const Msg& msg)
{
    setupConnection();
    return m_selfSocket->send(msg);
}

mu::async::Channel<Msg> IpcChannel::msgReceived() const
{
    return m_selfSocket->msgReceived();
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

void IpcChannel::onDisconected()
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

QList<Meta> IpcChannel::instances() const
{
    return m_selfSocket->instances();
}

mu::async::Notification IpcChannel::instancesChanged() const
{
    return m_selfSocket->instancesChanged();
}
