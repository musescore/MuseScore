/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "propertiespanelmodule.h"

#include "qml/MuseScore/PropertiesPanel/internal/propertiespanelpopupcontroller.h"

using namespace mu::propertiespanel;
using namespace muse::modularity;

static const std::string mname("propertiespanel");

std::string PropertiesPanelModule::moduleName() const
{
    return mname;
}

IContextSetup* PropertiesPanelModule::newContext(const ContextPtr& ctx) const
{
    return new PropertiesPanelContext(ctx);
}

void PropertiesPanelContext::registerExports()
{
    m_popupController = std::make_shared<PropertiesPanelPopupController>(iocContext());
    ioc()->registerExport<IPropertiesPanelPopupController>(mname, m_popupController);
}

void PropertiesPanelContext::onInit(const muse::IApplication::RunMode&)
{
    m_popupController->init();
}
