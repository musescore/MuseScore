/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "updatestubmodule.h"

#include "modularity/ioc.h"

#include "updateconfigurationstub.h"
#include "updatescenariostub.h"
#include "updateservicestub.h"

using namespace muse::update;
using namespace muse::modularity;

static std::shared_ptr<UpdateScenarioStub> s_scenario = std::make_shared<UpdateScenarioStub>();
static std::shared_ptr<UpdateServiceStub> s_service = std::make_shared<UpdateServiceStub>();
static std::shared_ptr<UpdateConfigurationStub> s_configuration = std::make_shared<UpdateConfigurationStub>();

std::string UpdateModule::moduleName() const
{
    return "update_stub";
}

void UpdateModule::registerExports()
{
    ioc()->registerExport<IUpdateScenario>(moduleName(), s_scenario);
    ioc()->registerExport<IAppUpdateService>(moduleName(), s_service);
    ioc()->registerExport<IUpdateConfiguration>(moduleName(), s_configuration);
}
