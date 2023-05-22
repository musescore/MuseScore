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
#include "cloudmodule.h"

#include <QQmlEngine>
#include "modularity/ioc.h"
#include "ui/iinteractiveuriregister.h"
#include "ui/iuiengine.h"

#include "internal/cloudservice.h"
#include "internal/cloudconfiguration.h"
#include "view/accountmodel.h"

using namespace mu::cloud;
using namespace mu::modularity;

static void cloud_init_qrc()
{
    Q_INIT_RESOURCE(cloud);
}

std::string CloudModule::moduleName() const
{
    return "cloud";
}

void CloudModule::registerExports()
{
    m_cloudConfiguration = std::make_shared<CloudConfiguration>();
    m_cloudService = std::make_shared<CloudService>();

    ioc()->registerExport<ICloudConfiguration>(moduleName(), m_cloudConfiguration);
    ioc()->registerExport<IAuthorizationService>(moduleName(), m_cloudService);
    ioc()->registerExport<ICloudProjectsService>(moduleName(), m_cloudService);
}

void CloudModule::resolveImports()
{
    auto ir = ioc()->resolve<ui::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("musescore://cloud/requireauthorization"), "MuseScore/Cloud/RequireAuthorizationDialog.qml");
    }
}

void CloudModule::registerResources()
{
    cloud_init_qrc();
}

void CloudModule::registerUiTypes()
{
    qmlRegisterType<AccountModel>("MuseScore.Cloud", 1, 0, "AccountModel");

    modularity::ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(cloud_QML_IMPORT);
}

void CloudModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (mode != framework::IApplication::RunMode::GuiApp) {
        return;
    }

    m_cloudConfiguration->init();
    m_cloudService->init();
}
