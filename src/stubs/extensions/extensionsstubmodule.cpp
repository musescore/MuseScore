//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "extensionsstubmodule.h"

#include "extensionsconfigurationstub.h"
#include "extensionsservicestub.h"
#include "extensionunpackerstub.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

using namespace mu::extensions;
using namespace mu::framework;

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
