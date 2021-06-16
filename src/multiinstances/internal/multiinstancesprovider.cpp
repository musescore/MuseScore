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
#include "settings.h"
#include "log.h"

using namespace mu::mi;
using namespace mu::ipc;
using namespace mu::framework;

static const mu::UriQuery DEV_SHOW_INFO_URI("musescore://devtools/multiinstances/info?sync=false&modal=false");
static const QString METHOD_SCORE_IS_OPENED("SCORE_IS_OPENED");
static const QString METHOD_ACTIVATE_WINDOW_WITH_SCORE("ACTIVATE_WINDOW_WITH_SCORE");

static const QString METHOD_SETTINGS_BEGIN_TRANSACTION("SETTINGS_BEGIN_TRANSACTION");
static const QString METHOD_SETTINGS_COMMIT_TRANSACTION("SETTINGS_COMMIT_TRANSACTION");
static const QString METHOD_SETTINGS_ROLLBACK_TRANSACTION("SETTINGS_ROLLBACK_TRANSACTION");
static const QString METHOD_SETTINGS_SET_VALUE("SETTINGS_SET_VALUE");

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

    m_ipcChannel->msgReceived().onReceive(this, [this](const Msg& msg) { onMsg(msg); });
    m_ipcChannel->instancesChanged().onNotify(this, [this]() { m_instancesChanged.notify(); });

    m_ipcChannel->connect();
}

void MultiInstancesProvider::onMsg(const Msg& msg)
{
    LOGI() << msg.method;

#define CHECK_ARGS_COUNT(c) IF_ASSERT_FAILED(msg.args.count() >= c) { return; }

    // Score opening
    if (msg.type == MsgType::Request && msg.method == METHOD_SCORE_IS_OPENED) {
        CHECK_ARGS_COUNT(1);
        io::path scorePath = io::path(msg.args.at(0));
        bool isOpened = fileScoreController()->isScoreOpened(scorePath);
        m_ipcChannel->response(METHOD_SCORE_IS_OPENED, { QString::number(isOpened) }, msg.srcID);
    } else if (msg.method == METHOD_ACTIVATE_WINDOW_WITH_SCORE) {
        CHECK_ARGS_COUNT(1);
        io::path scorePath = io::path(msg.args.at(0));
        bool isOpened = fileScoreController()->isScoreOpened(scorePath);
        if (isOpened) {
            mainWindow()->requestShowOnFront();
        }
    }
    // Settings
    else if (msg.method == METHOD_SETTINGS_BEGIN_TRANSACTION) {
        settings()->beginTransaction();
    } else if (msg.method == METHOD_SETTINGS_COMMIT_TRANSACTION) {
        settings()->commitTransaction();
    } else if (msg.method == METHOD_SETTINGS_ROLLBACK_TRANSACTION) {
        settings()->rollbackTransaction();
    } else if (msg.method == METHOD_SETTINGS_SET_VALUE) {
        CHECK_ARGS_COUNT(3);
        Settings::Key key("", msg.args.at(0).toStdString());
        Val val(msg.args.at(1).toStdString());
        val.setType(static_cast<Val::Type>(msg.args.at(2).toInt()));
        settings()->setValue(key, val);
    }
}

bool MultiInstancesProvider::isScoreAlreadyOpened(const io::path& scorePath) const
{
    int ret = 0;
    m_ipcChannel->syncRequestToAll(METHOD_SCORE_IS_OPENED, { scorePath.toQString() }, [&ret](const QStringList& args) {
        IF_ASSERT_FAILED(args.count() > 0) {
            return false;
        }
        ret = args.at(0).toInt();
        if (ret) {
            return true;
        }

        return false;
    });
    return ret;
}

void MultiInstancesProvider::activateWindowForScore(const io::path& scorePath)
{
    mainWindow()->requestShowOnBack();
    m_ipcChannel->broadcast(METHOD_ACTIVATE_WINDOW_WITH_SCORE, { scorePath.toQString() });
}

void MultiInstancesProvider::settingsBeginTransaction()
{
    m_ipcChannel->broadcast(METHOD_SETTINGS_BEGIN_TRANSACTION);
}

void MultiInstancesProvider::settingsCommitTransaction()
{
    m_ipcChannel->broadcast(METHOD_SETTINGS_COMMIT_TRANSACTION);
}

void MultiInstancesProvider::settingsRollbackTransaction()
{
    m_ipcChannel->broadcast(METHOD_SETTINGS_ROLLBACK_TRANSACTION);
}

void MultiInstancesProvider::settingsSetValue(const std::string& key, const Val& value)
{
    QStringList args;
    args << QString::fromStdString(key);
    args << value.toQString();
    args << QString::number(static_cast<int>(value.type()));
    m_ipcChannel->broadcast(METHOD_SETTINGS_SET_VALUE, args);
}

const std::string& MultiInstancesProvider::selfID() const
{
    return m_selfID;
}

std::vector<InstanceMeta> MultiInstancesProvider::instances() const
{
    std::vector<InstanceMeta> ret;
    QList<ID> ints = m_ipcChannel->instances();
    for (const ID& id : qAsConst(ints)) {
        InstanceMeta im;
        im.id = id.toStdString();
        ret.push_back(std::move(im));
    }

    return ret;
}

mu::async::Notification MultiInstancesProvider::instancesChanged() const
{
    return m_instancesChanged;
}
