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
#include "convertermodule.h"

#include "modularity/ioc.h"
#include "internal/convertercontroller.h"

#include "global/api/iapiregister.h"
#include "api/converterapi.h"

using namespace muse::modularity;
using namespace mu::converter;

static const std::string mname("converter");

std::string ConverterModule::moduleName() const
{
    return mname;
}

void ConverterModule::registerApi()
{
    using namespace muse::api;

    auto api = globalIoc()->resolve<IApiRegister>(mname);
    if (api) {
        api->regApiCreator(mname, "MuseApi.Converter", new ApiCreator<api::ConverterApi>());
    }
}

muse::modularity::IContextSetup* ConverterModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new ConverterModuleContext(ctx);
}

void ConverterModuleContext::registerExports()
{
    ioc()->registerExport<IConverterController>(mname, new ConverterController(iocContext()));
}
