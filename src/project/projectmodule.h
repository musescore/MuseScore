/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_PROJECT_PROJECTMODULE_H
#define MU_PROJECT_PROJECTMODULE_H

#include <memory>

#include "modularity/imodulesetup.h"

namespace mu::project {
class ProjectConfiguration;
class ProjectActionsController;
class RecentFilesController;
class ProjectAutoSaver;
class ProjectModule : public muse::modularity::IModuleSetup
{
public:

    std::string moduleName() const override;
    void registerExports() override;
    void resolveImports() override;
    void registerResources() override;
    void registerUiTypes() override;
    void onInit(const muse::IApplication::RunMode& mode) override;

private:
    std::shared_ptr<ProjectConfiguration> m_configuration;
    std::shared_ptr<ProjectActionsController> m_actionsController;
    std::shared_ptr<RecentFilesController> m_recentFilesController;
    std::shared_ptr<ProjectAutoSaver> m_projectAutoSaver;
};
}

#endif // MU_PROJECT_PROJECTMODULE_H
