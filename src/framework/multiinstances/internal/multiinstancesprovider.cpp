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

#include <QProcess>
#include <QCoreApplication>
#include <QTimer>
#include <QEventLoop>

#include "types/uri.h"
#include "settings.h"
#include "log.h"

using namespace muse;
using namespace muse::mi;
using namespace muse::ipc;
using namespace muse::actions;

static const muse::UriQuery DEV_SHOW_INFO_URI("muse://devtools/multiinstances/info?sync=false&modal=false");
static const QString METHOD_PROJECT_IS_OPENED("PROJECT_IS_OPENED");
static const QString METHOD_ACTIVATE_WINDOW_WITH_PROJECT("ACTIVATE_WINDOW_WITH_PROJECT");
static const QString METHOD_IS_WITHOUT_PROJECT("IS_WITHOUT_PROJECT");
static const QString METHOD_ACTIVATE_WINDOW_WITHOUT_PROJECT("METHOD_ACTIVATE_WINDOW_WITHOUT_PROJECT");

static const muse::Uri PREFERENCES_URI("muse://preferences");
static const QString METHOD_PREFERENCES_IS_OPENED("PREFERENCES_IS_OPENED");
static const QString METHOD_ACTIVATE_WINDOW_WITH_OPENED_PREFERENCES("ACTIVATE_WINDOW_WITH_OPENED_PREFERENCES");
static const QString METHOD_SETTINGS_BEGIN_TRANSACTION("SETTINGS_BEGIN_TRANSACTION");
static const QString METHOD_SETTINGS_COMMIT_TRANSACTION("SETTINGS_COMMIT_TRANSACTION");
static const QString METHOD_SETTINGS_ROLLBACK_TRANSACTION("SETTINGS_ROLLBACK_TRANSACTION");
static const QString METHOD_SETTINGS_RESET("SETTINGS_RESET");
static const QString METHOD_SETTINGS_SET_VALUE("SETTINGS_SET_VALUE");
static const QString METHOD_QUIT("METHOD_QUIT");
static const QString METHOD_QUIT_WITH_RESTART_LAST_INSTANCE("METHOD_QUIT_WITH_RESTART_LAST_INSTANCE");
static const QString METHOD_QUIT_WITH_RUNING_INSTALLATION("METHOD_QUIT_WITH_RUNING_INSTALLATION");
static const QString METHOD_QUITED("METHOD_QUITED");
static const QString METHOD_RESOURCE_CHANGED("RESOURCE_CHANGED");

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

    // Project opening
    if (msg.type == MsgType::Request && msg.method == METHOD_PROJECT_IS_OPENED) {
        CHECK_ARGS_COUNT(1);
        io::path_t scorePath = io::path_t(msg.args.at(0));
        bool isOpened = false;
        if (projectProvider()) {
            isOpened = projectProvider()->isProjectOpened(scorePath);
        } else {
            LOGW() << "Unable perform request, not found implementation of IProjectProvider";
        }
        m_ipcChannel->response(METHOD_PROJECT_IS_OPENED, { QString::number(isOpened) }, msg.srcID);
    } else if (msg.method == METHOD_ACTIVATE_WINDOW_WITH_PROJECT) {
        CHECK_ARGS_COUNT(1);
        io::path_t scorePath = io::path_t(msg.args.at(0));
        bool isOpened = false;
        if (projectProvider()) {
            isOpened = projectProvider()->isProjectOpened(scorePath);
        } else {
            LOGW() << "Unable perform request, not found implementation of IProjectProvider";
        }
        if (isOpened) {
            mainWindow()->requestShowOnFront();
        }
    } else if (msg.type == MsgType::Request && msg.method == METHOD_IS_WITHOUT_PROJECT) {
        bool isAnyOpened = false;
        if (projectProvider()) {
            isAnyOpened = projectProvider()->isAnyProjectOpened();
        } else {
            LOGW() << "Unable perform request, not found implementation of IProjectProvider";
        }
        m_ipcChannel->response(METHOD_IS_WITHOUT_PROJECT, { QString::number(!isAnyOpened) }, msg.srcID);
    } else if (msg.method == METHOD_ACTIVATE_WINDOW_WITHOUT_PROJECT) {
        bool isAnyOpened = false;
        if (projectProvider()) {
            isAnyOpened = projectProvider()->isAnyProjectOpened();
        } else {
            LOGW() << "Unable perform request, not found implementation of IProjectProvider";
        }
        if (!isAnyOpened) {
            mainWindow()->requestShowOnFront();
            if (msg.args.count() > 0 && !msg.args.at(0).isEmpty()) {
                dispatcher()->dispatch(msg.args.at(0).toStdString(), ActionData::make_arg1<bool>(false));
            }
        }
        m_ipcChannel->response(METHOD_ACTIVATE_WINDOW_WITH_PROJECT, { QString::number(!isAnyOpened) }, msg.srcID);
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
    } else if (msg.method == METHOD_SETTINGS_RESET) {
        settings()->reset(true, true, false);
    } else if (msg.method == METHOD_SETTINGS_SET_VALUE) {
        CHECK_ARGS_COUNT(3);
        Settings::Key key("", msg.args.at(0).toStdString());
        Val val(msg.args.at(1).toStdString());
        val.setType(static_cast<Val::Type>(msg.args.at(2).toInt()));
        settings()->setLocalValue(key, val);
    } else if (msg.method == METHOD_QUIT) {
        dispatcher()->dispatch("quit", ActionData::make_arg1<bool>(false));
    } else if (msg.method == METHOD_QUIT_WITH_RESTART_LAST_INSTANCE) {
        dispatcher()->dispatch("restart");
    } else if (msg.method == METHOD_QUIT_WITH_RUNING_INSTALLATION) {
        CHECK_ARGS_COUNT(1);
        dispatcher()->dispatch("quit", ActionData::make_arg2<bool, std::string>(false, msg.args.at(0).toStdString()));
    } else if (msg.method == METHOD_QUITED) {
        m_ipcChannel->response(METHOD_QUITED, { }, msg.srcID);
    } else if (msg.method == METHOD_RESOURCE_CHANGED) {
        resourceChanged().send(msg.args.at(0).toStdString());
    }
}

bool MultiInstancesProvider::isProjectAlreadyOpened(const io::path_t& projectPath) const
{
    if (!isInited()) {
        return false;
    }

    int ret = 0;
    m_ipcChannel->syncRequestToAll(METHOD_PROJECT_IS_OPENED, { projectPath.toQString() }, [&ret](const QStringList& args, const ID&) {
        IF_ASSERT_FAILED(!args.empty()) {
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

void MultiInstancesProvider::activateWindowWithProject(const io::path_t& projectPath)
{
    if (!isInited()) {
        return;
    }

#ifndef Q_OS_MAC
    // On macOS, this is not desirable, since raising the main window of the other instance works properly without this
    mainWindow()->requestShowOnBack();
#endif

    m_ipcChannel->broadcast(METHOD_ACTIVATE_WINDOW_WITH_PROJECT, { projectPath.toQString() });
}

bool MultiInstancesProvider::isHasAppInstanceWithoutProject() const
{
    if (!isInited()) {
        return false;
    }

    bool ret = false;
    m_ipcChannel->syncRequestToAll(METHOD_IS_WITHOUT_PROJECT, {}, [&ret](const QStringList& args, const ID&) {
        IF_ASSERT_FAILED(!args.empty()) {
            return false;
        }
        if (args.at(0).toInt()) {
            ret = true;
            return true;
        }
        return false;
    });
    return ret;
}

void MultiInstancesProvider::activateWindowWithoutProject(const QStringList& args)
{
    if (!isInited()) {
        return;
    }

#ifndef Q_OS_MAC
    // On macOS, this is not desirable, since raising the main window of the other instance works properly without this
    mainWindow()->requestShowOnBack();
#endif

    ID idWithNoProject;
    m_ipcChannel->syncRequestToAll(METHOD_IS_WITHOUT_PROJECT, {}, [&idWithNoProject](const QStringList& retArgs, const ID& srcId) {
        IF_ASSERT_FAILED(!retArgs.empty()) {
            return false;
        }
        if (retArgs.at(0).toInt()) {
            idWithNoProject = srcId;
            return true;
        }
        return false;
    });
    if (!idWithNoProject.isEmpty()) {
        m_ipcChannel->response(METHOD_ACTIVATE_WINDOW_WITHOUT_PROJECT, args, idWithNoProject);
        return;
    }
}

bool MultiInstancesProvider::openNewAppInstance(const QStringList& args)
{
    if (!isInited()) {
        return false;
    }

    QList<ipc::ID> currentApps = m_ipcChannel->instances();

    QString appPath = QCoreApplication::applicationFilePath();
    bool ok = QProcess::startDetached(appPath, args);
    if (ok) {
        LOGI() << "success start: " << appPath << ", args: " << args;
    } else {
        LOGE() << "failed start: " << appPath << ", args: " << args;
        return ok;
    }

    auto sleep = [](int msec) {
        QTimer timer;
        QEventLoop loop;
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(msec);
        loop.exec();
    };

    auto waitNewInstance = [this, sleep, currentApps](int waitMs, int count) {
        for (int i = 0; i < count; ++i) {
            sleep(waitMs);
            QList<ipc::ID> apps = m_ipcChannel->instances();
            for (const ipc::ID& id : apps) {
                if (!currentApps.contains(id)) {
                    LOGI() << "created new instance with ID: " << id;
                    return true;
                }
            }
        }
        return false;
    };

    //! NOTE Waiting for a new instance to be created
    ok = waitNewInstance(100, 50);
    if (!ok) {
        LOGE() << "we didn't wait for registration and response from the new instance";
    }

    return ok;
}

bool MultiInstancesProvider::isPreferencesAlreadyOpened() const
{
    if (!isInited()) {
        return false;
    }

    int ret = 0;
    m_ipcChannel->syncRequestToAll(METHOD_PREFERENCES_IS_OPENED, {}, [&ret](const QStringList& args, const ID&) {
        IF_ASSERT_FAILED(!args.empty()) {
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

#ifndef Q_OS_MAC
    // On macOS, this is not desirable, since raising the main window of the other instance works properly without this
    mainWindow()->requestShowOnBack();
#endif

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

void MultiInstancesProvider::settingsReset()
{
    if (!isInited()) {
        return;
    }

    m_ipcChannel->broadcast(METHOD_SETTINGS_RESET);
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

muse::ipc::IpcLock* MultiInstancesProvider::lock(const std::string& name)
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

void MultiInstancesProvider::notifyAboutResourceChanged(const std::string& name)
{
    if (!isInited()) {
        return;
    }

    QStringList args;
    args << QString::fromStdString(name);
    m_ipcChannel->broadcast(METHOD_RESOURCE_CHANGED, args);
}

async::Channel<std::string> MultiInstancesProvider::resourceChanged()
{
    return m_resourceChanged;
}

const std::string& MultiInstancesProvider::selfID() const
{
    return m_selfID;
}

bool MultiInstancesProvider::isMainInstance() const
{
    return m_ipcChannel ? m_ipcChannel->isServer() : false;
}

std::vector<InstanceMeta> MultiInstancesProvider::instances() const
{
    std::vector<InstanceMeta> ret;
    QList<ipc::ID> ints = m_ipcChannel->instances();
    for (const ipc::ID& id : std::as_const(ints)) {
        InstanceMeta im;
        im.id = id.toStdString();
        ret.push_back(std::move(im));
    }

    return ret;
}

async::Notification MultiInstancesProvider::instancesChanged() const
{
    return m_instancesChanged;
}

void MultiInstancesProvider::notifyAboutInstanceWasQuited()
{
    if (!isInited()) {
        return;
    }

    m_ipcChannel->broadcast(METHOD_QUITED);
}

void MultiInstancesProvider::quitForAll()
{
    if (!isInited()) {
        return;
    }

    m_ipcChannel->broadcast(METHOD_QUIT);
}

void MultiInstancesProvider::quitAllAndRestartLast()
{
    if (!isInited()) {
        return;
    }

    m_ipcChannel->broadcast(METHOD_QUIT_WITH_RESTART_LAST_INSTANCE);
}

void MultiInstancesProvider::quitAllAndRunInstallation(const io::path_t& installerPath)
{
    if (!isInited()) {
        return;
    }

    QStringList args;
    args << installerPath.toQString();
    m_ipcChannel->broadcast(METHOD_QUIT_WITH_RUNING_INSTALLATION, args);
}
