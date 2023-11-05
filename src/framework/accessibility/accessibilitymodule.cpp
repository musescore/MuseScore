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
#include "accessibilitymodule.h"

#include <QtQml>

#include "modularity/ioc.h"
#include "log.h"

#include "internal/accessibilitycontroller.h"
#include "internal/accessibilityconfiguration.h"
#include "internal/qaccessibleinterfaceregister.h"

using namespace mu::accessibility;
using namespace mu::modularity;

std::string AccessibilityModule::moduleName() const
{
    return "accessibility";
}

void AccessibilityModule::registerExports()
{
    m_configuration = std::make_shared<AccessibilityConfiguration>();

    ioc()->registerExport<IAccessibilityConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IAccessibilityController>(moduleName(), std::make_shared<AccessibilityController>());
    ioc()->registerExport<IQAccessibleInterfaceRegister>(moduleName(), new QAccessibleInterfaceRegister());
}

void AccessibilityModule::resolveImports()
{
    auto accr = ioc()->resolve<IQAccessibleInterfaceRegister>(moduleName());
    if (accr) {
        accr->registerInterfaceGetter("QQuickWindow", AccessibilityController::accessibleInterface);
        accr->registerInterfaceGetter("mu::accessibility::AccessibleObject", AccessibleObject::accessibleInterface);
    }
}

void AccessibilityModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (mode != framework::IApplication::RunMode::GuiApp) {
        return;
    }

    m_configuration->init();
}
