/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited and others
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
#include "audio/engine/isynthresolver.h"

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

static const std::string mname("musesampler");

std::string MuseSamplerModule::moduleName() const
{
    return mname;
}

void MuseSamplerModule::registerExports()
{
    m_configuration = std::make_shared<MuseSamplerConfiguration>(globalCtx());
    m_actionController = std::make_shared<MuseSamplerActionController>(globalCtx());
    m_resolver = std::make_shared<MuseSamplerResolver>();

    globalIoc()->registerExport<IMuseSamplerConfiguration>(mname, m_configuration);
    globalIoc()->registerExport<IMuseSamplerInfo>(mname, m_resolver);
}

void MuseSamplerModule::resolveImports()
{
    auto synthResolver = globalIoc()->resolve<synth::ISynthResolver>(mname);

    if (synthResolver) {
        synthResolver->registerResolver(AudioSourceType::MuseSampler, m_resolver);
    }
}

void MuseSamplerModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
    m_resolver->init();
    m_actionController->init(m_resolver);

    auto pr = globalIoc()->resolve<muse::diagnostics::IDiagnosticsPathsRegister>(mname);
    if (pr) {
        pr->reg("musesampler", m_configuration->libraryPath());
    }
}

void MuseSamplerModule::onDeinit()
{
    m_resolver->deinit();
}

IContextSetup* MuseSamplerModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new MuseSamplerContext(ctx);
}

void MuseSamplerContext::resolveImports()
{
    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(mname);
    if (ar) {
        ar->reg(std::make_shared<MuseSamplerUiActions>());
    }
}
