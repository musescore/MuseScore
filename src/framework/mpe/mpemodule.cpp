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

#include "mpemodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "internal/articulationprofilesrepository.h"
#include "view/articulationsprofileeditormodel.h"

using namespace muse;
using namespace muse::modularity;
using namespace muse::mpe;

static void mpe_init_qrc()
{
    Q_INIT_RESOURCE(mpe);
}

std::string MpeModule::moduleName() const
{
    return "mpe";
}

void MpeModule::registerExports()
{
    m_profilesRepository = std::make_shared<ArticulationProfilesRepository>(iocContext());

    ioc()->registerExport<IArticulationProfilesRepository>(moduleName(), m_profilesRepository);
}

void MpeModule::registerUiTypes()
{
    qmlRegisterType<ArticulationsProfileEditorModel>("Muse.Mpe", 1, 0, "ArticulationsProfileEditorModel");

    ioc()->resolve<muse::ui::IUiEngine>(moduleName())->addSourceImportPath(muse_mpe_QML_IMPORT);
}

void MpeModule::registerResources()
{
    mpe_init_qrc();
}
