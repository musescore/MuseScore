/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "multiinstances/imultiinstancesprovider.h"

#include "modularity/ioc.h"
#include "global/iapplication.h"

//! NOTE Just for demonstration
namespace mu::app {
class MultiWindowProvider : public muse::mi::IMultiInstancesProvider
{
    muse::Inject<muse::IApplication> application = { nullptr };
public:

    // Project opening
    bool isProjectAlreadyOpened(const muse::io::path_t& projectPath) const override { return false; }
    void activateWindowWithProject(const muse::io::path_t& projectPath) override {}
    bool isHasAppInstanceWithoutProject() const override { return false; }
    void activateWindowWithoutProject(const QStringList& args = { }) override {}
    bool openNewAppInstance(const QStringList& args) override;

    // Settings
    bool isPreferencesAlreadyOpened() const override { return false; }
    void activateWindowWithOpenedPreferences() const override {}
    void settingsBeginTransaction() override {}
    void settingsCommitTransaction() override {}
    void settingsRollbackTransaction() override {}
    void settingsReset() override {}
    void settingsSetValue(const std::string& key, const muse::Val& value) override {}

    // Resources (files)
    bool lockResource(const std::string& name) override { return false; }
    bool unlockResource(const std::string& name) override { return false; }
    void notifyAboutResourceChanged(const std::string& name) override { }
    muse::async::Channel<std::string> resourceChanged() override { return {}; }

    // Instances info
    const std::string& selfID() const override { return std::string(); }
    bool isMainInstance() const override { return false; }
    virtual std::vector<muse::mi::InstanceMeta> instances() const override { return {}; }
    virtual muse::async::Notification instancesChanged() const override { return muse::async::Notification(); }

    void notifyAboutInstanceWasQuited() override {}

    // Quit for all
    void quitForAll() override {}
    void quitAllAndRestartLast() override {}
    void quitAllAndRunInstallation(const muse::io::path_t& installerPath) override {}
};
}
