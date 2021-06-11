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
#include "multiinstancesprovider.h"

#include "uri.h"

#include "log.h"

using namespace mu::mi;
using namespace mu::ipc;

static const mu::UriQuery DEV_SHOW_INFO_URI("musescore://devtools/multiinstances/info?sync=false&modal=false");

MultiInstancesProvider::~MultiInstancesProvider()
{
    delete m_ipcChannel;
}

void MultiInstancesProvider::init()
{
    dispatcher()->reg(this, "multiinstances-dev-show-info", [this]() {
        if (!interactive()->isOpened(DEV_SHOW_INFO_URI.uri()).val) {
            interactive()->open(DEV_SHOW_INFO_URI);
        }
    });

    m_ipcChannel = new IpcChannel();
    m_selfID = m_ipcChannel->selfID().toStdString();

    m_ipcChannel->msgReceived().onReceive(this, [this](const Msg& msg) {
        onMsg(msg);
    });

    m_ipcChannel->instancesChanged().onNotify(this, [this]() {
        m_instancesChanged.notify();
    });

    m_ipcChannel->connect();

    m_timeout.setSingleShot(true);
    QObject::connect(&m_timeout, &QTimer::timeout, [this]() {
        LOGI() << "timeout";
        m_loop.quit();
    });
}

void MultiInstancesProvider::onMsg(const Msg& msg)
{
    LOGI() << msg.method;

    if (m_onMsg) {
        m_onMsg(msg);
    }

    if (msg.type == MsgType::Request && msg.method == "score_is_opened") {
        Msg answer;
        answer.destID = ipc::BROADCAST_ID;
        answer.type = MsgType::Response;
        answer.method = "score_is_opened";
        answer.args << QString::number(0);
        m_ipcChannel->send(answer);
    }
}

bool MultiInstancesProvider::isScoreAlreadyOpened(const io::path& scorePath) const
{
    //! NOTE Temporary solution, I will think how do this better

    int total = m_ipcChannel->instances().count();
    if (total < 2) {
        LOGD() << "only one instance";
        return false;
    }

    Msg msg;
    msg.destID = ipc::BROADCAST_ID;
    msg.type = MsgType::Request;
    msg.method = "score_is_opened";
    msg.args << scorePath.toQString();

    total -= 1;
    int recived = 0;
    bool ret = false;
    m_onMsg = [this, total, &recived, &ret](const Msg& msg) {
        if (!(msg.type == MsgType::Response && msg.method == "score_is_opened")) {
            return;
        }

        ++recived;
        ret = msg.args.at(0).toInt();
        if (ret || recived == total) {
            m_timeout.stop();
            m_loop.quit();
        }
    };

    LOGI() << "send Request, selfID: " << m_ipcChannel->selfID();

    m_ipcChannel->send(msg);

    m_timeout.start(500);
    m_loop.exec();

    m_onMsg = nullptr;

    return ret;
}

void MultiInstancesProvider::activateWindowForScore(const io::path& scorePath)
{
    Q_UNUSED(scorePath);
}

const std::string& MultiInstancesProvider::selfID() const
{
    return m_selfID;
}

std::vector<InstanceMeta> MultiInstancesProvider::instances() const
{
    std::vector<InstanceMeta> ret;
    QList<Meta> ints = m_ipcChannel->instances();
    for (const Meta& m : qAsConst(ints)) {
        InstanceMeta im;
        im.id = m.id.toStdString();
        ret.push_back(std::move(im));
    }

    return ret;
}

mu::async::Notification MultiInstancesProvider::instancesChanged() const
{
    return m_instancesChanged;
}
