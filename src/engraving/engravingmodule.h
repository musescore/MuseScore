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
#ifndef MU_ENGRAVING_ENGRAVINGMODULE_H
#define MU_ENGRAVING_ENGRAVINGMODULE_H

#include <memory>

#include "modularity/imodulesetup.h"

namespace mu::engraving {
class EngravingConfiguration;
class EngravingFontsProvider;
class EngravingModule : public muse::modularity::IModuleSetup
{
public:
    std::string moduleName() const override;

    void registerExports() override;
    void resolveImports() override;
    void registerApi() override;
    void registerResources() override;
    void registerUiTypes() override;
    void onInit(const muse::IApplication::RunMode& mode) override;
    void onDestroy() override;

private:

    std::shared_ptr<EngravingConfiguration> m_configuration;
    std::shared_ptr<EngravingFontsProvider> m_engravingfonts;
};
}

#endif // MU_ENGRAVING_ENGRAVINGMODULE_H
