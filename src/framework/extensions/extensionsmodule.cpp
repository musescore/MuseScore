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

#include "internal/extensionsprovider.h"
#include "internal/extensionsconfigure.h"

#include "devtools/devextensionslistmodel.h"

#include "log.h"

using namespace mu::extensions;
using namespace mu::modularity;

std::string ExtensionsModule::moduleName() const
{
    return "extensions";
}

void ExtensionsModule::registerExports()
{
    m_extensionsProvider = std::make_shared<ExtensionsProvider>();

    ioc()->registerExport<IExtensionsProvider>(moduleName(), m_extensionsProvider);
    ioc()->registerExport<IExtensionsConfigure>(moduleName(), new ExtensionsConfigure());
}

void ExtensionsModule::registerUiTypes()
{
    qmlRegisterType<DevExtensionsListModel>("Muse.Extensions", 1, 0, "DevExtensionsListModel");
}

void ExtensionsModule::onInit(const IApplication::RunMode& mode)
{
    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }
}
