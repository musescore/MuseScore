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
#include "internal/extensionscontroller.h"
#include "internal/extensionunpacker.h"
#include "view/extensionlistmodel.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

using namespace mu::extensions;

static ExtensionsConfiguration* m_extensionsConfiguration = new ExtensionsConfiguration();
static ExtensionsController* m_extensionsController = new ExtensionsController();

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
    framework::ioc()->registerExport<IExtensionsConfiguration>(moduleName(), m_extensionsConfiguration);
    framework::ioc()->registerExport<IExtensionsController>(moduleName(), m_extensionsController);
    framework::ioc()->registerExport<IExtensionUnpacker>(moduleName(), new ExtensionUnpacker());
}

void ExtensionsModule::registerResources()
{
    extensions_init_qrc();
}

void ExtensionsModule::registerUiTypes()
{
    qmlRegisterType<ExtensionListModel>("MuseScore.Extensions", 1, 0, "ExtensionListModel");
    qmlRegisterUncreatableType<ExtensionStatus>("MuseScore.Extensions", 1, 0, "ExtensionStatus", "Cannot create an ExtensionStatus");

    framework::ioc()->resolve<framework::IUiEngine>(moduleName())->addSourceImportPath(extensions_QML_IMPORT);
}

void ExtensionsModule::onInit()
{
    m_extensionsController->init();
    m_extensionsConfiguration->init();
}
