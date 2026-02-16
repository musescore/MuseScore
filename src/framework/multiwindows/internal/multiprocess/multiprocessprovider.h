/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include <map>

#include "../../imultiwindowsprovider.h"
#include "imultiprocessprovider.h"

#include "ipc/ipcchannel.h"
#include "ipc/ipclock.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "async/asyncable.h"
#include "async/notification.h"
#include "interactive/iinteractive.h"
#include "ui/imainwindow.h"
#include "../../iprojectprovider.h"

namespace muse::mi {
class MultiProcessProvider : public IMultiWindowsProvider, public IMultiProcessProvider, public Contextable, public actions::Actionable,
    public async::Asyncable
{
    ContextInject<muse::actions::IActionsDispatcher> dispatcher = { this };
    ContextInject<IInteractive> interactive = { this };
    ContextInject<muse::ui::IMainWindow> mainWindow = { this };

    //! NOTE May be missing because it must be implemented outside the framework
    ContextInject<IProjectProvider> projectProvider = { this };

public:
    MultiProcessProvider(const modularity::ContextPtr& iocCtx)
        : Contextable(iocCtx)
    {
    }

    ~MultiProcessProvider();

    void init();

    // Contexts info
    int windowCount() const override;
    bool isFirstWindow() const override;

    // Project opening
    bool isProjectAlreadyOpened(const io::path_t& projectPath) const override;
    void activateWindowWithProject(const io::path_t& projectPath) override;
    bool isHasWindowWithoutProject() const override;
    void activateWindowWithoutProject(const QStringList& args) override;
    bool openNewWindow(const QStringList& args) override;

    // Settings
    bool isPreferencesAlreadyOpened() const override;
    void activateWindowWithOpenedPreferences() const override;
    void settingsBeginTransaction() override;
    void settingsCommitTransaction() override;
    void settingsRollbackTransaction() override;
    void settingsReset() override;
    void settingsSetValue(const std::string& key, const Val& value) override;

    // Resources (files)
    bool lockResource(const std::string& name) override;
    bool unlockResource(const std::string& name) override;
    void notifyAboutResourceChanged(const std::string& name) override;
    async::Channel<std::string> resourceChanged() override;

    // Quit for all
    void notifyAboutWindowWasQuited() override;
    void quitForAll() override;
    void quitAllAndRestartLast() override;
    void quitAllAndRunInstallation(const io::path_t& installerPath) override;

    // IMultiProcessProvider
    const std::string& selfID() const override;
    bool isMainInstance() const override;
    std::vector<InstanceMeta> instances() const override;
    async::Notification instancesChanged() const override;

private:

    bool isInited() const;

    void onMsg(const muse::ipc::Msg& msg);

    muse::ipc::IpcLock* lock(const std::string& name);

    muse::ipc::IpcChannel* m_ipcChannel = nullptr;
    std::string m_selfID;

    async::Notification m_instancesChanged;
    async::Channel<std::string> m_resourceChanged;

    std::map<std::string, muse::ipc::IpcLock*> m_locks;
};
}
