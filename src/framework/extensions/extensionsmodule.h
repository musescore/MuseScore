/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#include <string>

#include "modularity/imodulesetup.h"

namespace muse::extensions {
class ExtensionsProvider;
class ExtensionsActionController;
class ExtensionsConfiguration;
class ExtensionsExecPointsRegister;
class ExtensionsModule : public modularity::IModuleSetup
{
public:

    std::string moduleName() const override;
    void registerExports() override;
    void registerResources() override;
    void registerUiTypes() override;
    void resolveImports() override;
    void registerApi() override;
    void onInit(const IApplication::RunMode& mode) override;
    void onDelayedInit() override;

private:

    std::shared_ptr<ExtensionsConfiguration> m_configuration;
    std::shared_ptr<ExtensionsProvider> m_provider;
    std::shared_ptr<ExtensionsActionController> m_actionController;
    std::shared_ptr<ExtensionsExecPointsRegister> m_execPointsRegister;
    bool m_extensionsLoaded = false;
};
}
