/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "actionsmodule.h"

#include "modularity/ioc.h"
#include "internal/actionsdispatcher.h"

#include "global/api/iapiregister.h"
#include "api/dispatcherapi.h"

using namespace muse::actions;
using namespace muse::actions::api;
using namespace muse::modularity;

static const std::string mname("actions");

std::string ActionsModule::moduleName() const
{
    return mname;
}

void ActionsModule::registerApi()
{
    using namespace muse::api;

    auto api = globalIoc()->resolve<IApiRegister>(mname);
    if (api) {
        api->regApiCreator(mname, "MuseInternal.Dispatcher", new ApiCreator<DispatcherApi>());
    }
}

IContextSetup* ActionsModule::newContext(const modularity::ContextPtr& ctx) const
{
    return new ActionsModuleContext(ctx);
}

void ActionsModuleContext::registerExports()
{
    ioc()->registerExport<IActionsDispatcher>(mname, new ActionsDispatcher());
}
