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

#include <QAccessible>

#include "modularity/ioc.h"

#include "iaccessibleapprootobject.h"
#include "internal/accessibilitycontroller.h"
#include "internal/accessibleapprootobject.h"
#include "internal/accessiblestub.h"
#include "internal/qaccessibleinterfaceregister.h"
#include "iqaccessibleinterfaceregister.h"

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
    globalIoc()->registerExport<IQAccessibleInterfaceRegister>(mname, new QAccessibleInterfaceRegister());
    globalIoc()->registerExport<IAccessibleAppRootObject>(mname, new AccessibleAppRootObject());
}

void AccessibilityModule::resolveImports()
{
    auto accr = globalIoc()->resolve<IQAccessibleInterfaceRegister>(mname);
    if (accr) {
#ifndef Q_OS_LINUX // https://github.com/musescore/MuseScore/pull/32258#issuecomment-3972545361
        accr->registerInterfaceGetter("QQuickWindow", AccessibilityController::accessibleInterface);
#endif
        accr->registerInterfaceGetter("muse::accessibility::AccessibleObject", AccessibleObject::accessibleInterface);
        accr->registerInterfaceGetter("muse::accessibility::AccessibleAppRootObject", AccessibleAppRootObject::accessibleInterface);
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

static QAccessibleInterface* accessibleFactory(const QString& classname, QObject* object)
{
    auto accr = globalIoc()->resolve<IQAccessibleInterfaceRegister>("accessibility");
    if (accr) {
        auto interfaceGetter = accr->interfaceGetter(classname);
        if (interfaceGetter) {
            return interfaceGetter(object);
        }
    }

    return AccessibleStub::accessibleInterface(object);
}

void AccessibilityModule::onInit(const IApplication::RunMode&)
{
    QAccessible::installFactory(accessibleFactory);

    auto appRoot = globalIoc()->resolve<IAccessibleAppRootObject>(mname);
    if (appRoot) {
        appRoot->init();
    }
}

IContextSetup* AccessibilityModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new AccessibilityContext(ctx);
}

// Context
void AccessibilityContext::registerExports()
{
    //! FIXME AccessibilityController has a global component and a contextual component.
    // It probably needs to be split into two separate classes.
    m_controller = std::make_shared<AccessibilityController>(iocContext());
    ioc()->registerExport<IAccessibilityController>(mname, m_controller);
}

void AccessibilityContext::onPreInit(const IApplication::RunMode&)
{
    m_controller->setAccessibilityEnabled(true);
}

void AccessibilityContext::onDeinit()
{
    m_controller->deinit();
}
