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
#ifndef MU_PALETTE_PALETTEMODULE_H
#define MU_PALETTE_PALETTEMODULE_H

#include <memory>

#include "modularity/imodulesetup.h"

namespace mu::palette {
class PaletteProvider;
class PaletteActionsController;
class PaletteUiActions;
class PaletteConfiguration;
class PaletteWorkspaceSetup;
class PaletteModule : public muse::modularity::IModuleSetup
{
public:
    std::string moduleName() const override;

    void registerExports() override;
    void resolveImports() override;

    void registerResources() override;
    void registerUiTypes() override;

    void onInit(const muse::IApplication::RunMode& mode) override;
    void onAllInited(const muse::IApplication::RunMode& mode) override;
    void onDeinit() override;

private:

    std::shared_ptr<PaletteProvider> m_paletteProvider;
    std::shared_ptr<PaletteActionsController> m_actionsController;
    std::shared_ptr<PaletteUiActions> m_paletteUiActions;
    std::shared_ptr<PaletteConfiguration> m_configuration;
    std::shared_ptr<PaletteWorkspaceSetup> m_paletteWorkspaceSetup;
};
}

#endif // MU_PALETTE_PALETTEMODULE_H
