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
#include "extensionsstubmodule.h"

#include "extensionsconfigurationstub.h"
#include "extensionsservicestub.h"
#include "extensionunpackerstub.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

using namespace mu::extensions;
using namespace mu::modularity;

static void extensions_init_qrc()
{
    Q_INIT_RESOURCE(extensions);
}

std::string ExtensionsStubModule::moduleName() const
{
    return "extensions_stub";
}

void ExtensionsStubModule::registerExports()
{
    ioc()->registerExport<IExtensionsConfiguration>(moduleName(), new ExtensionsConfigurationStub());
    ioc()->registerExport<IExtensionsService>(moduleName(), new ExtensionsServiceStub());
    ioc()->registerExport<IExtensionUnpacker>(moduleName(), new ExtensionUnpackerStub());
}

void ExtensionsStubModule::registerResources()
{
    extensions_init_qrc();
}

void ExtensionsStubModule::registerUiTypes()
{
    ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(extensions_QML_IMPORT);
}
