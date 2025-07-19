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
#ifndef MUSE_IPC_IPCCHANNEL_H
#define MUSE_IPC_IPCCHANNEL_H

#include <functional>

#include <QString>
#include <QList>

#include "ipc.h"

#include "async/asyncable.h"
#include "async/channel.h"
#include "async/notification.h"

namespace muse::ipc {
//! NOTE Inter-Process Communication Channel
class IpcSocket;
class IpcServer;
class IpcChannel : public async::Asyncable
{
public:
    IpcChannel();
    ~IpcChannel();

    const ID& selfID() const;

    bool isServer() const;

    void connect();

    bool send(const Msg& msg);
    async::Channel<Msg> msgReceived() const;

    void response(const QString& method, const QStringList& args, const ID& destID);
    void broadcast(const QString& method, const QStringList& args = {});

    using OnReceived = std::function<bool (const QStringList&, const ID& srcID)>;
    Code syncRequestToAll(const QString& method, const QStringList& args, const OnReceived& onReceived);

    QList<ID> instances() const;
    async::Notification instancesChanged() const;

private:

    void setupConnection();
    void onDisconnected();
    void onSocketMsgReceived(const Msg& msg);

    IpcSocket* m_selfSocket = nullptr;
    IpcServer* m_server = nullptr;
    std::function<void(const Msg&)> m_msgCallback;
    async::Channel<Msg> m_msgReceived;
};
}

#endif // MUSE_IPC_IPCCHANNEL_H
