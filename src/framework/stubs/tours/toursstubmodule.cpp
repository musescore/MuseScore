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
#include "toursstubmodule.h"

#include "modularity/ioc.h"

#include "toursconfigurationstub.h"
#include "toursservicestub.h"
#include "toursproviderstub.h"

using namespace muse::tours;
using namespace muse::modularity;

static const std::string mname("tours_stub");

std::string ToursModule::moduleName() const
{
    return mname;
}

void ToursModule::registerExports()
{
    globalIoc()->registerExport<IToursConfiguration>(mname, new ToursConfigurationStub());
}

IContextSetup* ToursModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new ToursContext(ctx);
}

void ToursContext::registerExports()
{
    ioc()->registerExport<IToursService>(mname, new ToursServiceStub());
    ioc()->registerExport<IToursProvider>(mname, new ToursProviderStub());
}
