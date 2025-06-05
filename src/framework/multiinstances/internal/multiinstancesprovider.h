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
#ifndef MUSE_MI_MULTIINSTANCESPROVIDER_H
#define MUSE_MI_MULTIINSTANCESPROVIDER_H

#include <map>

#include "../imultiinstancesprovider.h"

#include "ipc/ipcchannel.h"
#include "ipc/ipclock.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "iinteractive.h"
#include "async/asyncable.h"
#include "ui/imainwindow.h"
#include "../iprojectprovider.h"

namespace muse::mi {
class MultiInstancesProvider : public IMultiInstancesProvider, public Injectable, public actions::Actionable, public async::Asyncable
{
    Inject<muse::actions::IActionsDispatcher> dispatcher = { this };
    Inject<IInteractive> interactive = { this };
    Inject<muse::ui::IMainWindow> mainWindow = { this };

    //! NOTE May be missing because it must be implemented outside the framework
    Inject<IProjectProvider> projectProvider = { this };

public:
    MultiInstancesProvider(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}
    ~MultiInstancesProvider();

    void init();

    // Project opening
    bool isProjectAlreadyOpened(const io::path_t& projectPath) const override;
    void activateWindowWithProject(const io::path_t& projectPath) override;
    bool isHasAppInstanceWithoutProject() const override;
    void activateWindowWithoutProject(const QStringList& args) override;
    bool openNewAppInstance(const QStringList& args) override;

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

    // Instances info
    const std::string& selfID() const override;
    bool isMainInstance() const override;
    std::vector<InstanceMeta> instances() const override;
    async::Notification instancesChanged() const override;

    void notifyAboutInstanceWasQuited() override;

    // Quit for all
    void quitForAll() override;
    void quitAllAndRestartLast() override;
    void quitAllAndRunInstallation(const io::path_t& installerPath) override;

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

#endif // MUSE_MI_MULTIINSTANCESPROVIDER_H
