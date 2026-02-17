/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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
#include "accessibilitymodule.h"

#include "modularity/ioc.h"

#include "internal/accessibilitycontroller.h"
#include "internal/accessibilityconfiguration.h"
#include "internal/qaccessibleinterfaceregister.h"

#include "global/api/iapiregister.h"
#include "api/accessibilityapi.h"

using namespace muse::accessibility;
using namespace muse::modularity;

std::string AccessibilityModule::moduleName() const
{
    return "accessibility";
}

void AccessibilityModule::registerExports()
{
    m_configuration = std::make_shared<AccessibilityConfiguration>(globalCtx());
    m_controller = std::make_shared<AccessibilityController>(globalCtx());

    globalIoc()->registerExport<IAccessibilityConfiguration>(moduleName(), m_configuration);
    globalIoc()->registerExport<IAccessibilityController>(moduleName(), m_controller);
    globalIoc()->registerExport<IQAccessibleInterfaceRegister>(moduleName(), new QAccessibleInterfaceRegister());
}

void AccessibilityModule::resolveImports()
{
    auto accr = globalIoc()->resolve<IQAccessibleInterfaceRegister>(moduleName());
    if (accr) {
#ifdef Q_OS_MAC
        accr->registerInterfaceGetter("QQuickWindow", AccessibilityController::accessibleInterface);
#endif
        accr->registerInterfaceGetter("muse::accessibility::AccessibleObject", AccessibleObject::accessibleInterface);
    }
}

void AccessibilityModule::registerApi()
{
    using namespace muse::api;

    auto api = globalIoc()->resolve<IApiRegister>(moduleName());
    if (api) {
        api->regApiCreator(moduleName(), "MuseInternal.Accessibility", new ApiCreator<api::AccessibilityApi>());
    }
}

void AccessibilityModule::onPreInit(const IApplication::RunMode&)
{
    m_controller->setAccessibilityEnabled(true);
}

void AccessibilityModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
}
