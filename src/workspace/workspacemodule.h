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
#ifndef MU_WORKSPACE_WORKSPACEMODULE_H
#define MU_WORKSPACE_WORKSPACEMODULE_H

#include <memory>

#include "modularity/imodulesetup.h"

namespace mu::workspace {
class WorkspaceManager;
class WorkspaceConfiguration;
class WorkspaceActionController;
class WorkspacesDataProvider;
class WorkspaceModule : public modularity::IModuleSetup
{
public:
    std::string moduleName() const override;
    void registerExports() override;
    void resolveImports() override;
    void registerUiTypes() override;
    void registerResources() override;
    void onInit(const framework::IApplication::RunMode& mode) override;
    void onDeinit() override;

private:

    std::shared_ptr<WorkspaceManager> m_manager;
    std::shared_ptr<WorkspaceConfiguration> m_configuration;
    std::shared_ptr<WorkspaceActionController> m_actionController;
    std::shared_ptr<WorkspacesDataProvider> m_provider;
};
}

#endif // MU_WORKSPACE_WORKSPACEMODULE_H
