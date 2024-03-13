/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#include "extensionsmodule.h"

#include <QtQml>

#include "modularity/ioc.h"

#include "ui/iinteractiveuriregister.h"

#include "internal/extensionsprovider.h"
#include "internal/extensionsconfiguration.h"

#include "internal/legacy/api/qmlpluginapi.h"

#include "view/extensionbuilder.h"
#include "view/extensionsuiengine.h"

#include "devtools/devextensionslistmodel.h"

#include "log.h"

using namespace mu::extensions;
using namespace mu::modularity;

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
    m_extensionsProvider = std::make_shared<ExtensionsProvider>();

    ioc()->registerExport<IExtensionsProvider>(moduleName(), m_extensionsProvider);
    ioc()->registerExport<IExtensionsConfiguration>(moduleName(), new ExtensionsConfiguration());
    ioc()->registerExport<IExtensionsUiEngine>(moduleName(), new ExtensionsUiEngine());
}

void ExtensionsModule::registerResources()
{
    extensions_init_qrc();
}

void ExtensionsModule::registerUiTypes()
{
    qmlRegisterType<ExtensionBuilder>("Muse.Extensions", 1, 0, "ExtensionBuilder");
    qmlRegisterType<DevExtensionsListModel>("Muse.Extensions", 1, 0, "DevExtensionsListModel");

    legacy::QmlPluginApi::registerQmlTypes();
}

void ExtensionsModule::resolveImports()
{
    auto ir = ioc()->resolve<ui::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("musescore://extensions/viewer"), "Muse/Extensions/ExtensionViewerDialog.qml");
    }
}

void ExtensionsModule::onInit(const IApplication::RunMode& mode)
{
    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }
}
