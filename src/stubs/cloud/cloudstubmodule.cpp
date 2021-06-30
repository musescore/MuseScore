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
#include "cloudstubmodule.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "authorizationservicestub.h"

using namespace mu::cloud;
using namespace mu::modularity;

static void cloud_init_qrc()
{
    Q_INIT_RESOURCE(cloud);
}

std::string CloudStubModule::moduleName() const
{
    return "cloud_stub";
}

void CloudStubModule::registerExports()
{
    ioc()->registerExport<IAuthorizationService>(moduleName(), new AuthorizationServiceStub());
}

void CloudStubModule::registerResources()
{
    cloud_init_qrc();
}

void CloudStubModule::registerUiTypes()
{
    ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(cloud_QML_IMPORT);
}
