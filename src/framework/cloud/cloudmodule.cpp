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

#ifdef MUSE_MODULE_CLOUD_MUSESCORECOM
#include "musescorecom/musescorecomservice.h"
#endif
#include "audiocom/audiocomservice.h"
#include "internal/cloudconfiguration.h"

#include "view/cloudsmodel.h"
#include "view/musescorecomauthorizationmodel.h"

#include "cloudqmltypes.h"

using namespace muse;
using namespace muse::cloud;
using namespace muse::modularity;

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
    m_cloudConfiguration = std::make_shared<CloudConfiguration>(iocContext());
    ioc()->registerExport<ICloudConfiguration>(moduleName(), m_cloudConfiguration);
#ifdef MUSE_MODULE_CLOUD_MUSESCORECOM
    m_museScoreComService = std::make_shared<MuseScoreComService>(iocContext());
    ioc()->registerExport<IMuseScoreComService>(moduleName(), m_museScoreComService);
#endif
    m_audioComService = std::make_shared<AudioComService>(iocContext());
    ioc()->registerExport<IAudioComService>(moduleName(), m_audioComService);
}

void CloudModule::resolveImports()
{
    auto ir = ioc()->resolve<ui::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("muse://cloud/requireauthorization"), "Muse/Cloud/RequireAuthorizationDialog.qml");
    }
}

void CloudModule::registerResources()
{
    cloud_init_qrc();
}

void CloudModule::registerUiTypes()
{
    qmlRegisterType<CloudsModel>("Muse.Cloud", 1, 0, "CloudsModel");
    qmlRegisterType<MuseScoreComAuthorizationModel>("Muse.Cloud", 1, 0, "MuseScoreComAuthorizationModel");

    qmlRegisterUncreatableType<QMLCloudVisibility>("Muse.Cloud", 1, 0, "CloudVisibility",
                                                   "Not creatable as it is an enum type");

    qmlRegisterUncreatableType<QMLSaveToCloudResponse>("Muse.Cloud", 1, 0, "SaveToCloudResponse",
                                                       "Not creatable as it is an enum type");

    ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(muse_cloud_QML_IMPORT);
}

void CloudModule::onInit(const IApplication::RunMode& mode)
{
    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }

    m_cloudConfiguration->init();
#ifdef MUSE_MODULE_CLOUD_MUSESCORECOM
    m_museScoreComService->init();
#endif
    m_audioComService->init();
}
