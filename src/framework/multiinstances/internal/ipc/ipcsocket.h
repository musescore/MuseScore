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
#ifndef MUSE_IPC_IPCSOCKET_H
#define MUSE_IPC_IPCSOCKET_H

#include "ipc.h"

#include "async/channel.h"
#include "async/notification.h"
#include "async/asyncable.h"

class QLocalSocket;

namespace muse::ipc {
class IpcLock;
class IpcSocket : public async::Asyncable
{
public:
    IpcSocket() = default;
    ~IpcSocket();

    const ID& selfID() const;

    bool connect(const QString& serverName);
    async::Notification disconnected();

    bool send(Msg msg);
    async::Channel<Msg> msgReceived() const;

    QList<ID> instances() const;
    async::Notification instancesChanged() const;

private:

    void onReadyRead();
    void onDataReceived(const QByteArray& data);
    void onIpcMsg(const Msg& receivedMsg);

    mutable ID m_selfID = 0;
    IpcLock* m_lock = nullptr;
    QLocalSocket* m_socket = nullptr;
    async::Notification m_disconnected;
    async::Channel<Msg> m_msgReceived;

    QList<ID> m_instances;
    async::Notification m_instancesChanged;
};
}

#endif // MUSE_IPC_IPCSOCKET_H
