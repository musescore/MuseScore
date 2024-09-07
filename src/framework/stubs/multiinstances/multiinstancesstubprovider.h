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
#ifndef MUSE_MI_MULTIINSTANCESSTUBPROVIDER_H
#define MUSE_MI_MULTIINSTANCESSTUBPROVIDER_H

#include "multiinstances/imultiinstancesprovider.h"

namespace muse::mi {
class MultiInstancesStubProvider : public IMultiInstancesProvider
{
public:
    MultiInstancesStubProvider() = default;

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
};
}

#endif // MUSE_MI_MULTIINSTANCESSTUBPROVIDER_H
