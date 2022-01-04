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
#ifndef MU_MI_MULTIINSTANCESSTUBPROVIDER_H
#define MU_MI_MULTIINSTANCESSTUBPROVIDER_H

#include "multiinstances/imultiinstancesprovider.h"

namespace mu::mi {
class MultiInstancesStubProvider : public IMultiInstancesProvider
{
public:
    MultiInstancesStubProvider() = default;

    // Score opening
    bool isProjectAlreadyOpened(const io::path& scorePath) const override;
    void activateWindowWithProject(const io::path& scorePath) override;
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

    // Instances info
    const std::string& selfID() const override;
    std::vector<InstanceMeta> instances() const override;
    async::Notification instancesChanged() const override;

    // Quit for all
    void quitForAll() override;
};
}

#endif // MU_MI_MULTIINSTANCESSTUBPROVIDER_H
