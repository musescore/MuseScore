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

static const std::string mname("accessibility");

std::string AccessibilityModule::moduleName() const
{
    return mname;
}

void AccessibilityModule::registerExports()
{
    m_configuration = std::make_shared<AccessibilityConfiguration>(globalCtx());

    globalIoc()->registerExport<IAccessibilityConfiguration>(mname, m_configuration);
    globalIoc()->registerExport<IQAccessibleInterfaceRegister>(mname, new QAccessibleInterfaceRegister());
}

void AccessibilityModule::resolveImports()
{
    auto accr = globalIoc()->resolve<IQAccessibleInterfaceRegister>(mname);
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

    auto api = globalIoc()->resolve<IApiRegister>(mname);
    if (api) {
        api->regApiCreator(mname, "MuseInternal.Accessibility", new ApiCreator<api::AccessibilityApi>());
    }
}

void AccessibilityModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
}

IContextSetup* AccessibilityModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new AccessibilityContext(ctx);
}

// Context
void AccessibilityContext::registerExports()
{
    m_controller = std::make_shared<AccessibilityController>(iocContext());
    ioc()->registerExport<IAccessibilityController>(mname, m_controller);
}

void AccessibilityContext::onPreInit(const IApplication::RunMode&)
{
    m_controller->setAccessibilityEnabled(true);
}
