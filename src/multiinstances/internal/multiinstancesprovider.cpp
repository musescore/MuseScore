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

static const mu::Uri PREFERENCES_URI("musescore://preferences");
static const QString METHOD_PREFERENCES_IS_OPENED("PREFERENCES_IS_OPENED");
static const QString METHOD_ACTIVATE_WINDOW_WITH_OPENED_PREFERENCES("ACTIVATE_WINDOW_WITH_OPENED_PREFERENCES");
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

bool MultiInstancesProvider::isInited() const
{
    return m_ipcChannel != nullptr;
}

void MultiInstancesProvider::onMsg(const Msg& msg)
{
    LOGI() << msg.method;

#define CHECK_ARGS_COUNT(c) IF_ASSERT_FAILED(msg.args.count() >= c) { return; }

    // Score opening
    if (msg.type == MsgType::Request && msg.method == METHOD_SCORE_IS_OPENED) {
        CHECK_ARGS_COUNT(1);
        io::path scorePath = io::path(msg.args.at(0));
        bool isOpened = fileScoreController()->isProjectOpened(scorePath);
        m_ipcChannel->response(METHOD_SCORE_IS_OPENED, { QString::number(isOpened) }, msg.srcID);
    } else if (msg.method == METHOD_ACTIVATE_WINDOW_WITH_SCORE) {
        CHECK_ARGS_COUNT(1);
        io::path scorePath = io::path(msg.args.at(0));
        bool isOpened = fileScoreController()->isProjectOpened(scorePath);
        if (isOpened) {
            mainWindow()->requestShowOnFront();
        }
    }
    // Settings
    else if (msg.type == MsgType::Request && msg.method == METHOD_PREFERENCES_IS_OPENED) {
        bool isOpened = interactive()->isOpened(PREFERENCES_URI).val;
        m_ipcChannel->response(METHOD_PREFERENCES_IS_OPENED, { QString::number(isOpened) }, msg.srcID);
    } else if (msg.method == METHOD_ACTIVATE_WINDOW_WITH_OPENED_PREFERENCES) {
        bool isOpened = interactive()->isOpened(PREFERENCES_URI).val;
        if (isOpened) {
            mainWindow()->requestShowOnFront();
        }
    } else if (msg.method == METHOD_SETTINGS_BEGIN_TRANSACTION) {
        settings()->beginTransaction(false);
    } else if (msg.method == METHOD_SETTINGS_COMMIT_TRANSACTION) {
        settings()->commitTransaction(false);
    } else if (msg.method == METHOD_SETTINGS_ROLLBACK_TRANSACTION) {
        settings()->rollbackTransaction(false);
    } else if (msg.method == METHOD_SETTINGS_SET_VALUE) {
        CHECK_ARGS_COUNT(3);
        Settings::Key key("", msg.args.at(0).toStdString());
        Val val(msg.args.at(1).toStdString());
        val.setType(static_cast<Val::Type>(msg.args.at(2).toInt()));
        settings()->setLocalValue(key, val);
    }
}

bool MultiInstancesProvider::isScoreAlreadyOpened(const io::path& scorePath) const
{
    if (!isInited()) {
        return false;
    }

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

void MultiInstancesProvider::activateWindowWithScore(const io::path& scorePath)
{
    if (!isInited()) {
        return;
    }

    mainWindow()->requestShowOnBack();
    m_ipcChannel->broadcast(METHOD_ACTIVATE_WINDOW_WITH_SCORE, { scorePath.toQString() });
}

bool MultiInstancesProvider::isPreferencesAlreadyOpened() const
{
    if (!isInited()) {
        return false;
    }

    int ret = 0;
    m_ipcChannel->syncRequestToAll(METHOD_PREFERENCES_IS_OPENED, {}, [&ret](const QStringList& args) {
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

void MultiInstancesProvider::activateWindowWithOpenedPreferences() const
{
    if (!isInited()) {
        return;
    }

    mainWindow()->requestShowOnBack();
    m_ipcChannel->broadcast(METHOD_ACTIVATE_WINDOW_WITH_OPENED_PREFERENCES);
}

void MultiInstancesProvider::settingsBeginTransaction()
{
    if (!isInited()) {
        return;
    }

    m_ipcChannel->broadcast(METHOD_SETTINGS_BEGIN_TRANSACTION);
}

void MultiInstancesProvider::settingsCommitTransaction()
{
    if (!isInited()) {
        return;
    }

    m_ipcChannel->broadcast(METHOD_SETTINGS_COMMIT_TRANSACTION);
}

void MultiInstancesProvider::settingsRollbackTransaction()
{
    if (!isInited()) {
        return;
    }

    m_ipcChannel->broadcast(METHOD_SETTINGS_ROLLBACK_TRANSACTION);
}

void MultiInstancesProvider::settingsSetValue(const std::string& key, const Val& value)
{
    if (!isInited()) {
        return;
    }

    QStringList args;
    args << QString::fromStdString(key);
    args << value.toQString();
    args << QString::number(static_cast<int>(value.type()));
    m_ipcChannel->broadcast(METHOD_SETTINGS_SET_VALUE, args);
}

mu::ipc::IpcLock* MultiInstancesProvider::lock(const std::string& name)
{
    auto it = m_locks.find(name);
    if (it != m_locks.end()) {
        return it->second;
    }
    ipc::IpcLock* l = new ipc::IpcLock(QString::fromStdString(name));
    m_locks[name] = l;
    return l;
}

bool MultiInstancesProvider::lockResource(const std::string& name)
{
    return lock(name)->lock();
}

bool MultiInstancesProvider::unlockResource(const std::string& name)
{
    return lock(name)->unlock();
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
