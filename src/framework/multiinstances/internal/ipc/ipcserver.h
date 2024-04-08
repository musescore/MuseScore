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
#ifndef MUSE_IPC_IPCSERVER_H
#define MUSE_IPC_IPCSERVER_H

#include <QList>

#include "ipc.h"

class QLocalServer;
class QLocalSocket;
class QThread;

namespace muse::ipc {
class IpcLock;
class IpcServer
{
public:
    IpcServer() = default;
    ~IpcServer();

    bool listen(const QString& serverName);

private:

    struct IncomingSocket
    {
        ID id;
        QLocalSocket* socket = nullptr;
    };

    void onIncomingReadyRead(QLocalSocket* socket);
    void onIncomingInit(QLocalSocket* socket, const Msg& msg);
    void onIncomingWhoIs(QLocalSocket* socket, const Msg& msg);
    void onIncomingPing(QLocalSocket* socket, const Msg& msg);

    bool doSendToSocket(QLocalSocket* socket, const QByteArray& data);
    void sendToSocket(QLocalSocket* socket, const Msg& msg);
    void askWhoIs(QLocalSocket* socket);
    void sendMetaInfoToAllIncoming();

    void onDisconnected(QLocalSocket* socket);

    IncomingSocket& incomingSocket(QLocalSocket* socket);
    const IncomingSocket& incomingSocket(QLocalSocket* socket) const;

    IpcLock* m_lock = nullptr;
    QLocalServer* m_server = nullptr;
    QList<IncomingSocket> m_incomingSockets;
    QThread* m_thread = nullptr;
};
}

#endif // MUSE_IPC_IPCSERVER_H
