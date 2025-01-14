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

#ifndef MUSE_IPC_IPC_H
#define MUSE_IPC_IPC_H

#include <QString>
#include <QStringList>
#include <QByteArray>

#include <functional>

#include "muse_framework_config.h"

class QLocalSocket;

namespace muse::ipc {
#ifdef MUSE_APP_UNSTABLE
static const QString SERVER_NAME("musescore-app-ipc-" MUSE_APP_VERSION_MAJOR "-development");
#else
static const QString SERVER_NAME("musescore-app-ipc-" MUSE_APP_VERSION_MAJOR);
#endif

using ID = QString;

static const int TIMEOUT_MSEC(500);

static const ID BROADCAST_ID("broadcast");
static const ID DIRECT_SOCKET_ID("socket");
static const ID SERVER_ID("server");

static const QByteArray ACK("ipc_ack");
static const QString IPC_("ipc_");
static const QString IPC_INIT("ipc_init");
static const QString IPC_WHOIS("ipc_whois");
static const QString IPC_METAINFO("ipc_metainfo");
static const QString IPC_PING("ipc_ping");

static constexpr uint32_t MAX_PACKAGE_SIZE = 4096;

enum class Code {
    Undefined = -1,
    Success = 0,
    Timeout,
    AllAnswered
};

enum class MsgType {
    Undefined = 0,
    Notify,
    Request,
    Response
};

struct Msg
{
    QString srcID;
    QString destID;
    MsgType type = MsgType::Undefined;
    QString method;
    QStringList args;

    bool isValid() const { return type != MsgType::Undefined && !method.isEmpty(); }
};

void serialize(const Msg& msg, QByteArray& data);
void deserialize(const QByteArray& data, Msg& msg);

QString socketErrorToString(int err);

bool writeToSocket(QLocalSocket* socket, const QByteArray& data);
bool readFromSocket(QLocalSocket* socket, std::function<void(const QByteArray& data)> onPackageRead);
}

#endif // MUSE_IPC_IPC_H
