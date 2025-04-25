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
#pragma once

#include <memory>

#include "modularity/imodulesetup.h"

namespace mu::notation {
class NotationConfiguration;
class NotationActionController;
class NotationUiActions;
class MidiInputOutputController;
class InstrumentsRepository;
class EngravingFontsController;
class NotationModule : public muse::modularity::IModuleSetup
{
public:
    std::string moduleName() const override;

    void registerExports() override;
    void resolveImports() override;
    void registerResources() override;
    void registerUiTypes() override;
    void onInit(const muse::IApplication::RunMode& mode) override;

private:
    std::shared_ptr<NotationConfiguration> m_configuration;
    std::shared_ptr<NotationActionController> m_actionController;
    std::shared_ptr<NotationUiActions> m_notationUiActions;
    std::shared_ptr<MidiInputOutputController> m_midiInputOutputController;
    std::shared_ptr<InstrumentsRepository> m_instrumentsRepository;
    std::shared_ptr<EngravingFontsController> m_engravingFontsController;
};
}
