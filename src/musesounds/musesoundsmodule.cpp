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
#include "musesoundsmodule.h"

#include "modularity/ioc.h"
#include "framework/interactive/iinteractiveuriregister.h"

#include "internal/musesoundsconfiguration.h"
#include "internal/musesoundsrepository.h"

#include "internal/musesoundscheckupdatescenario.h"
#include "internal/musesoundscheckupdateservice.h"
#include "internal/musesamplercheckupdateservice.h"
#include "internal/musesamplercheckupdatescenario.h"

using namespace mu::musesounds;
using namespace muse;
using namespace muse::modularity;

static const std::string mname("musesounds");

std::string MuseSoundsModule::moduleName() const
{
    return mname;
}

void MuseSoundsModule::registerExports()
{
    m_configuration = std::make_shared<MuseSoundsConfiguration>();
    m_repository = std::make_shared<MuseSoundsRepository>();

    globalIoc()->registerExport<IMuseSoundsConfiguration>(mname, m_configuration);
    globalIoc()->registerExport<IMuseSoundsRepository>(mname, m_repository);
    globalIoc()->registerExport<IMuseSamplerCheckUpdateService>(mname, new MuseSamplerCheckUpdateService());
    globalIoc()->registerExport<IMuseSoundsCheckUpdateService>(mname, new MuseSoundsCheckUpdateService());
}

void MuseSoundsModule::resolveImports()
{
    auto ir = globalIoc()->resolve<interactive::IInteractiveUriRegister>(mname);
    if (ir) {
        ir->registerQmlUri(Uri("musescore://musesounds/musesoundsreleaseinfo"), "MuseScore.MuseSounds", "MuseSoundsReleaseInfoDialog");
    }
}

void MuseSoundsModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
    m_repository->init();
}

IContextSetup* MuseSoundsModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new MuseSoundsModuleContext(ctx);
}

void MuseSoundsModuleContext::registerExports()
{
    ioc()->registerExport<IMuseSoundsCheckUpdateScenario>(mname, new MuseSoundsCheckUpdateScenario(iocContext()));
    ioc()->registerExport<IMuseSamplerCheckUpdateScenario>(mname, new MuseSamplerCheckUpdateScenario(iocContext()));
}
