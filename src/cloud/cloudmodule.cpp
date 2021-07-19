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
#include "ui/iuiengine.h"
#include "ui/iuiactionsregister.h"

#include "internal/cloudservice.h"
#include "internal/cloudconfiguration.h"
#include "view/accountmodel.h"

using namespace mu::cloud;
using namespace mu::modularity;

static std::shared_ptr<CloudConfiguration> s_cloudConfiguration = std::make_shared<CloudConfiguration>();
static std::shared_ptr<CloudService> s_cloudService = std::make_shared<CloudService>();

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
    ioc()->registerExport<ICloudConfiguration>(moduleName(), s_cloudConfiguration);
    ioc()->registerExport<IAuthorizationService>(moduleName(), s_cloudService);
    ioc()->registerExport<IUploadingService>(moduleName(), s_cloudService);
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
    if (mode != framework::IApplication::RunMode::Editor) {
        return;
    }

    s_cloudConfiguration->init();
    s_cloudService->init();
}
