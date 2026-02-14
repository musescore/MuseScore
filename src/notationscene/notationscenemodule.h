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
class NotationSceneConfiguration;
class NotationActionController;
class NotationUiActions;
class MidiInputOutputController;
class NotationSceneModule : public muse::modularity::IModuleSetup
{
public:
    std::string moduleName() const override;
    void resolveImports() override;

    muse::modularity::IContextSetup* newContext(const muse::modularity::ContextPtr& ctx) const override;
};

class NotationSceneContext : public muse::modularity::IContextSetup
{
public:
    NotationSceneContext(const muse::modularity::ContextPtr& ctx)
        : muse::modularity::IContextSetup(ctx) {}

    void registerExports() override;
    void resolveImports() override;
    void onInit(const muse::IApplication::RunMode& mode) override;
    void onAllInited(const muse::IApplication::RunMode& mode) override;

private:
    std::shared_ptr<NotationSceneConfiguration> m_configuration;
    std::shared_ptr<NotationActionController> m_actionController;
    std::shared_ptr<NotationUiActions> m_notationUiActions;
    std::shared_ptr<MidiInputOutputController> m_midiInputOutputController;
};
}
