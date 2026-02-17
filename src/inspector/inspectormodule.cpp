/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "inspectormodule.h"

#include "qml/MuseScore/Inspector/internal/inspectorpopupcontroller.h"

using namespace mu::inspector;

std::string InspectorModule::moduleName() const
{
    return "inspector";
}

void InspectorModule::registerExports()
{
    m_popupController = std::make_shared<InspectorPopupController>(muse::modularity::globalCtx());

    globalIoc()->registerExport<IInspectorPopupController>(moduleName(), m_popupController);
}

void InspectorModule::onInit(const muse::IApplication::RunMode&)
{
    m_popupController->init();
}
