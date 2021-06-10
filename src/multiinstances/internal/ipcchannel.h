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
#ifndef MU_MI_IPCCHANNEL_H
#define MU_MI_IPCCHANNEL_H

#include <QString>
#include <QList>

#include "async/channel.h"
#include "async/notification.h"

class QLocalSocket;
class QLocalServer;
class QSystemSemaphore;
namespace mu::mi {
//! NOTE Inter-Process Communication Channel
class IpcChannel
{
public:
    IpcChannel() = default;
    ~IpcChannel();

    struct Meta
    {
        QString id = 0;
        bool isServer = false;

        bool isValid() const { return !id.isEmpty(); }
    };

    struct Msg
    {
        QString destID;
        int requestID = 0;
        QString method;
        QStringList args;
    };

    const QString& selfID() const;
    bool init();
    void ping();

    bool send(const Msg& msg);
    async::Channel<Msg> listen();

    QList<Meta> instances() const;
    async::Notification instancesChanged() const;

private:

    // common

    bool sendToSocket(QLocalSocket* socket, const Msg& msg) const;
    bool doSendToSocket(QLocalSocket* socket, const QByteArray& data) const;
    void doReadSocket(QLocalSocket* socket, QByteArray& data) const;

    void serialize(const Meta& meta, const Msg& msg, QByteArray& data) const;
    void deserialize(const QByteArray& data, Meta& meta, Msg& msg) const;

    // self socket
    bool checkConnectToServer();
    void onDataReceived(const QByteArray& data);
    void onIpcMsg(const Meta& receivedMeta, const Msg& receivedMsg);

    // me as server
    bool selfIsServer() const;
    bool makeServer();

    void onIncomingReadyRead(QLocalSocket* socket);
    void onDisconnected(QLocalSocket* socket);
    void askWhoIs(QLocalSocket* socket) const;

    void onIncomingInit(QLocalSocket* socket, const Meta& meta, const Msg& msg);
    void onIncomingWhoIs(QLocalSocket* socket, const Meta& meta, const Msg& msg);
    void onIncomingPing(QLocalSocket* socket, const Meta& meta, const Msg& msg);

    void sendMetaInfoToAllIncoming();

    struct IncomingSocket
    {
        Meta meta;
        QLocalSocket* socket = nullptr;
    };

    IncomingSocket& incomingSocket(QLocalSocket* socket);

    mutable QString m_selfID = 0;
    QLocalSocket* m_selfSocket = nullptr;

    QSystemSemaphore* m_serverLock = nullptr;
    QLocalServer* m_server = nullptr;

    QList<IncomingSocket> m_incomingSockets;

    async::Channel<Msg> m_listenCh;

    QList<Meta> m_instances;
    async::Notification m_instancesChanged;
};
}

#endif // MU_MI_IPCCHANNEL_H
