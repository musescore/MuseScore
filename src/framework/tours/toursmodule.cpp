/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#include "toursmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"

#include "internal/toursservice.h"
#include "internal/toursconfiguration.h"

#include "view/toursprovider.h"
#include "view/toursprovidermodel.h"

using namespace muse::tours;
using namespace muse::modularity;

static void tours_init_qrc()
{
    Q_INIT_RESOURCE(tours);
}

std::string ToursModule::moduleName() const
{
    return "tours";
}

void ToursModule::registerExports()
{
    m_service = std::make_shared<ToursService>(iocContext());
    m_configuration = std::make_shared<ToursConfiguration>(iocContext());
    m_provider = std::make_shared<ToursProvider>(iocContext());

    ioc()->registerExport<IToursService>(moduleName(), m_service);
    ioc()->registerExport<IToursConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IToursProvider>(moduleName(), m_provider);
}

void ToursModule::registerResources()
{
    tours_init_qrc();
}

void ToursModule::registerUiTypes()
{
    qmlRegisterType<ToursProviderModel>("Muse.Tours", 1, 0, "ToursProviderModel");
}

void ToursModule::onInit(const IApplication::RunMode& mode)
{
    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }

    m_service->init();
}
