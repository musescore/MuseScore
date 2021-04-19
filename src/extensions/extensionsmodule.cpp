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
#include "extensionsmodule.h"

#include <QQmlEngine>

#include "internal/extensionsconfiguration.h"
#include "internal/extensionsservice.h"
#include "internal/extensionunpacker.h"
#include "view/extensionlistmodel.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

using namespace mu::extensions;

static ExtensionsConfiguration* m_extensionsConfiguration = new ExtensionsConfiguration();
static ExtensionsService* m_extensionsService = new ExtensionsService();

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
    framework::ioc()->registerExport<IExtensionsService>(moduleName(), m_extensionsService);
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

    framework::ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(extensions_QML_IMPORT);
}

void ExtensionsModule::onInit(const framework::IApplication::RunMode& runMode)
{
    if (framework::IApplication::RunMode::Editor != runMode) {
        return;
    }

    m_extensionsService->init();
    m_extensionsConfiguration->init();
}
