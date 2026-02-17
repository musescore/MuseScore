/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "appupdatescenariostub.h"
#include "appupdateservicestub.h"

#include "updateconfigurationstub.h"

using namespace muse::update;
using namespace muse::modularity;

std::string UpdateModule::moduleName() const
{
    return "update_stub";
}

void UpdateModule::registerExports()
{
    globalIoc()->registerExport<IAppUpdateService>(moduleName(), std::make_shared<AppUpdateServiceStub>());
    globalIoc()->registerExport<IAppUpdateScenario>(moduleName(), std::make_shared<AppUpdateScenarioStub>());
    globalIoc()->registerExport<IUpdateConfiguration>(moduleName(), std::make_shared<UpdateConfigurationStub>());
}
