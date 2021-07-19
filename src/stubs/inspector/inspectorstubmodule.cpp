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
#include "inspectorstubmodule.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

using namespace mu::inspector;
using namespace mu::modularity;

static void inspector_init_qrc()
{
    Q_INIT_RESOURCE(inspector_resources);
}

std::string InspectorModule::moduleName() const
{
    return "inspector";
}

void InspectorModule::registerExports()
{
}

void InspectorModule::registerResources()
{
    inspector_init_qrc();
}

void InspectorModule::registerUiTypes()
{
    ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(inspector_QML_IMPORT);
}
