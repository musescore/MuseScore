/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "musesoundsstubmodule.h"

#include "modularity/ioc.h"

#include "musesoundscheckupdatescenariostub.h"
#include "musesoundscheckupdateservicestub.h"

using namespace mu::musesounds;
using namespace muse;
using namespace muse::modularity;

static std::shared_ptr<MuseSoundsCheckUpdateScenarioStub> s_museSoundsCheckUpdateScenario
    = std::make_shared<MuseSoundsCheckUpdateScenarioStub>();
static std::shared_ptr<MuseSoundsCheckUpdateServiceStub> s_museSoundsCheckUpdateService
    = std::make_shared<MuseSoundsCheckUpdateServiceStub>();

static void musesounds_init_qrc()
{
    Q_INIT_RESOURCE(musesounds);
}

std::string MuseSoundsModule::moduleName() const
{
    return "musesounds_stub";
}

void MuseSoundsModule::registerExports()
{
    ioc()->registerExport<IMuseSoundsCheckUpdateScenario>(moduleName(), s_museSoundsCheckUpdateScenario);
    ioc()->registerExport<IMuseSoundsCheckUpdateService>(moduleName(), s_museSoundsCheckUpdateService);
}

void MuseSoundsModule::registerResources()
{
    musesounds_init_qrc();
}
