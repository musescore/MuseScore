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

#include <memory>

#include "modularity/imodulesetup.h"

namespace muse::workspace {
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

    void onInit(const IApplication::RunMode& mode) override;
    void onDeinit() override;

    modularity::IContextSetup* newContext(const muse::modularity::ContextPtr& ctx) const override;

private:

    std::shared_ptr<WorkspaceConfiguration> m_configuration;
    std::shared_ptr<WorkspaceActionController> m_actionController;
    std::shared_ptr<WorkspaceManager> m_manager;
    std::shared_ptr<WorkspacesDataProvider> m_provider;
};

class WorkspaceContext : public modularity::IContextSetup
{
public:
    WorkspaceContext(const muse::modularity::ContextPtr& ctx)
        : modularity::IContextSetup(ctx) {}

    void registerExports() override;
    void onInit(const IApplication::RunMode& mode) override;
    void resolveImports() override;
    void onDeinit() override;

private:
    std::shared_ptr<WorkspaceActionController> m_actionController;
    std::shared_ptr<WorkspaceManager> m_manager;
    std::shared_ptr<WorkspacesDataProvider> m_provider;
};
}
