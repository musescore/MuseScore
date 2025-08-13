/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
#include "musesoundsmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "framework/ui/iinteractiveuriregister.h"

#include "internal/musesoundsconfiguration.h"
#include "internal/musesoundsrepository.h"

#include "internal/musesoundscheckupdatescenario.h"
#include "internal/musesoundscheckupdateservice.h"
#include "internal/musesamplercheckupdateservice.h"
#include "internal/musesamplercheckupdatescenario.h"

#include "view/musesoundslistmodel.h"

using namespace mu::musesounds;
using namespace muse;

static void musesounds_init_qrc()
{
    Q_INIT_RESOURCE(musesounds);
}

std::string MuseSoundsModule::moduleName() const
{
    return "musesounds";
}

void MuseSoundsModule::registerExports()
{
    m_configuration = std::make_shared<MuseSoundsConfiguration>(iocContext());
    m_repository = std::make_shared<MuseSoundsRepository>(iocContext());

    m_museSoundsCheckUpdateScenario = std::make_shared<MuseSoundsCheckUpdateScenario>(iocContext());
    m_museSoundsCheckUpdateService = std::make_shared<MuseSoundsCheckUpdateService>(iocContext());
    m_museSamplerCheckUpdateService = std::make_shared<MuseSamplerCheckUpdateService>(iocContext());
    m_museSamplerCheckUpdateScenario = std::make_shared<MuseSamplerCheckUpdateScenario>(iocContext());

    ioc()->registerExport<IMuseSoundsConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IMuseSoundsRepository>(moduleName(), m_repository);

    ioc()->registerExport<IMuseSoundsCheckUpdateScenario>(moduleName(), m_museSoundsCheckUpdateScenario);
    ioc()->registerExport<IMuseSoundsCheckUpdateService>(moduleName(), m_museSoundsCheckUpdateService);

    ioc()->registerExport<IMuseSamplerCheckUpdateService>(moduleName(), m_museSamplerCheckUpdateService);
    ioc()->registerExport<IMuseSamplerCheckUpdateScenario>(moduleName(), m_museSamplerCheckUpdateScenario);
}

void MuseSoundsModule::resolveImports()
{
    auto ir = ioc()->resolve<ui::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("musescore://musesounds/musesoundsreleaseinfo"), "MuseScore/MuseSounds/MuseSoundsReleaseInfoDialog.qml");
    }
}

void MuseSoundsModule::registerResources()
{
    musesounds_init_qrc();
}

void MuseSoundsModule::registerUiTypes()
{
    qmlRegisterType<MuseSoundsListModel>("MuseScore.MuseSounds", 1, 0, "MuseSoundsListModel");
}

void MuseSoundsModule::onInit(const IApplication::RunMode& mode)
{
    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }

    m_configuration->init();
    m_repository->init();
}
