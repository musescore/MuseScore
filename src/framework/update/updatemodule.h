/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

namespace muse::update {
class AppUpdateScenario;
class AppUpdateService;
class UpdateConfiguration;
class UpdateActionController;
class UpdateModule : public modularity::IModuleSetup
{
public:
    std::string moduleName() const override;
    void registerExports() override;
    void resolveImports() override;
    void onInit(const IApplication::RunMode& mode) override;

    modularity::IContextSetup* newContext(const muse::modularity::ContextPtr& ctx) const override;

private:
    std::shared_ptr<UpdateConfiguration> m_configuration;
};

class UpdateContext : public modularity::IContextSetup
{
public:
    UpdateContext(const muse::modularity::ContextPtr& ctx)
        : modularity::IContextSetup(ctx) {}

    void registerExports() override;
    void resolveImports() override;
    void onInit(const IApplication::RunMode& mode) override;

private:
    std::shared_ptr<AppUpdateScenario> m_appUpdateScenario;
    std::shared_ptr<AppUpdateService> m_appUpdateService;
    std::shared_ptr<UpdateActionController> m_actionController;
};
}
