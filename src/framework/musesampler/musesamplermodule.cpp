/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "musesamplermodule.h"

#include "modularity/ioc.h"
#include "audio/isynthresolver.h"

#include "ui/iuiactionsregister.h"

#include "imusesamplerinfo.h"
#include "internal/musesamplerconfiguration.h"
#include "internal/musesamplerresolver.h"
#include "internal/musesampleruiactions.h"
#include "internal/musesampleractioncontroller.h"

#include "diagnostics/idiagnosticspathsregister.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::modularity;
using namespace muse::musesampler;

std::string MuseSamplerModule::moduleName() const
{
    return "musesampler";
}

void MuseSamplerModule::registerExports()
{
    m_configuration = std::make_shared<MuseSamplerConfiguration>(iocContext());
    m_actionController = std::make_shared<MuseSamplerActionController>(iocContext());
    m_resolver = std::make_shared<MuseSamplerResolver>(iocContext());

    ioc()->registerExport<IMuseSamplerConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IMuseSamplerInfo>(moduleName(), m_resolver);
}

void MuseSamplerModule::resolveImports()
{
    auto synthResolver = ioc()->resolve<synth::ISynthResolver>(moduleName());

    if (synthResolver) {
        synthResolver->registerResolver(AudioSourceType::MuseSampler, m_resolver);
    }

    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<MuseSamplerUiActions>());
    }
}

void MuseSamplerModule::onInit(const IApplication::RunMode& mode)
{
    if (IApplication::RunMode::AudioPluginRegistration == mode) {
        return;
    }

    m_configuration->init();
    m_resolver->init();
    m_actionController->init([this]() {
        return m_resolver->reloadMuseSampler();
    });

    auto pr = ioc()->resolve<muse::diagnostics::IDiagnosticsPathsRegister>(moduleName());
    if (pr) {
        pr->reg("musesampler", m_configuration->userLibraryPath());
        pr->reg("musesampler fallback", m_configuration->fallbackLibraryPath());
    }
}
