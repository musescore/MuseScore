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
#ifndef MU_WORKSPACE_WORKSPACE_H
#define MU_WORKSPACE_WORKSPACE_H

#include "io/path.h"
#include "async/asyncable.h"
#include "iworkspace.h"
#include "workspacefile.h"

#include "modularity/ioc.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "global/iapplication.h"
#include "io/ifilesystem.h"
#include "iworkspaceconfiguration.h"

namespace muse::workspace {
class Workspace : public IWorkspace, public Injectable, public async::Asyncable
{
    Inject<mi::IMultiInstancesProvider> multiInstancesProvider = { this };
    Inject<IApplication> application = { this };
    Inject<io::IFileSystem> fileSystem = { this };
    Inject<IWorkspaceConfiguration> configuration = { this };

public:
    Workspace(const io::path_t& filePath, const modularity::ContextPtr& iocCtx);
    Workspace(const io::path_t& filePath, const Workspace* other, const modularity::ContextPtr& iocCtx);

    std::string name() const override;

    bool isBuiltin() const override;
    bool isEdited() const override;
    bool isNeedSave() const override;

    RetVal<QByteArray> rawData(const DataKey& key) const override;
    Ret setRawData(const DataKey& key, const QByteArray& data) override;

    void reset() override;
    void assignNewName(const std::string& newName) override;

    async::Notification reloadNotification() override;

    io::path_t filePath() const;
    bool isLoaded() const;
    Ret load();
    Ret save();

private:
    void reload();

    Ret doSave();

    std::string fileResourceName() const;

    io::path_t builtinWorkspacePath() const;

    void copyBuiltinWorkspaceToUserDir();

    std::shared_ptr<WorkspaceFile> m_file = nullptr;

    async::Notification m_reloadNotification;
};

using WorkspacePtr = std::shared_ptr<Workspace>;
}

#endif // MU_WORKSPACE_WORKSPACE_H
