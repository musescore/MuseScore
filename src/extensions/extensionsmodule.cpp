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
#include "extensionsmodule.h"

#include <QQmlEngine>

#include "internal/extensionsconfiguration.h"
#include "internal/extensionsservice.h"
#include "internal/extensionunpacker.h"
#include "view/extensionlistmodel.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

using namespace mu::extensions;
using namespace mu::framework;

static std::shared_ptr<ExtensionsConfiguration> s_configuration = std::make_shared<ExtensionsConfiguration>();
static std::shared_ptr<ExtensionsService> s_extensionsService = std::make_shared<ExtensionsService>();

static void extensions_init_qrc()
{
    Q_INIT_RESOURCE(extensions);
}

std::string ExtensionsModule::moduleName() const
{
    return "extensions";
}

void ExtensionsModule::registerExports()
{
    ioc()->registerExport<IExtensionsConfiguration>(moduleName(), s_configuration);
    ioc()->registerExport<IExtensionContentProvider>(moduleName(), s_configuration);
    ioc()->registerExport<IExtensionsService>(moduleName(), s_extensionsService);
    ioc()->registerExport<IExtensionUnpacker>(moduleName(), new ExtensionUnpacker());
}

void ExtensionsModule::registerResources()
{
    extensions_init_qrc();
}

void ExtensionsModule::registerUiTypes()
{
    qmlRegisterType<ExtensionListModel>("MuseScore.Extensions", 1, 0, "ExtensionListModel");
    qmlRegisterUncreatableType<ExtensionStatus>("MuseScore.Extensions", 1, 0, "ExtensionStatus", "Cannot create an ExtensionStatus");

    ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(extensions_QML_IMPORT);
}

void ExtensionsModule::onInit(const IApplication::RunMode& runMode)
{
    if (IApplication::RunMode::Editor != runMode) {
        return;
    }

    s_configuration->init();
    s_extensionsService->init();
}
