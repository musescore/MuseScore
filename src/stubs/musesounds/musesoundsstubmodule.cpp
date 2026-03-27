/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "musesoundsstubmodule.h"

#include "modularity/ioc.h"

#include "musesoundscheckupdatescenariostub.h"
#include "musesoundscheckupdateservicestub.h"

#include "musesamplercheckupdatescenariostub.h"
#include "musesamplercheckupdateservicestub.h"

using namespace mu::musesounds;
using namespace muse::modularity;

static const std::string mname("musesounds_stub");

std::string MuseSoundsModule::moduleName() const
{
    return mname;
}

void MuseSoundsModule::registerExports()
{
    globalIoc()->registerExport<IMuseSoundsCheckUpdateService>(mname, new MuseSoundsCheckUpdateServiceStub());
    globalIoc()->registerExport<IMuseSamplerCheckUpdateService>(mname, new MuseSamplerCheckUpdateServiceStub());
}

IContextSetup* MuseSoundsModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new MuseSoundsContext(ctx);
}

void MuseSoundsContext::registerExports()
{
    ioc()->registerExport<IMuseSoundsCheckUpdateScenario>(mname, new MuseSoundsCheckUpdateScenarioStub());
    ioc()->registerExport<IMuseSamplerCheckUpdateScenario>(mname, new MuseSamplerCheckUpdateScenarioStub());
}
