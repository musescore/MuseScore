/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

namespace mu::musesounds {
class MuseSoundsConfiguration;
class MuseSoundsRepository;
class MuseSoundsCheckUpdateScenario;
class MuseSoundsCheckUpdateService;
class MuseSamplerCheckUpdateService;
class MuseSoundsModule : public muse::modularity::IModuleSetup
{
public:
    std::string moduleName() const override;

    void registerExports() override;
    void resolveImports() override;
    void registerResources() override;
    void registerUiTypes() override;
    void onInit(const muse::IApplication::RunMode& mode) override;
    void onDelayedInit() override;

private:
    std::shared_ptr<MuseSoundsConfiguration> m_configuration;
    std::shared_ptr<MuseSoundsRepository> m_repository;
    std::shared_ptr<MuseSoundsCheckUpdateScenario> m_museSoundsCheckUpdateScenario;
    std::shared_ptr<MuseSoundsCheckUpdateService> m_museSoundsCheckUpdateService;
    std::shared_ptr<MuseSamplerCheckUpdateService> m_museSamplerCheckUpdateService;
};
}
