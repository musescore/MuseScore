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
#include "internal/accessibilityuiactions.h"
#include "internal/accessibilityconfiguration.h"
#include "dev/accessibledevmodel.h"

#include "ui/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"

using namespace mu::accessibility;
using namespace mu::modularity;

static std::shared_ptr<AccessibilityController> s_accessibilityController = std::make_shared<AccessibilityController>();

static void accessibility_init_qrc()
{
    Q_INIT_RESOURCE(accessibility);
}

std::string AccessibilityModule::moduleName() const
{
    return "accessibility";
}

void AccessibilityModule::registerExports()
{
    ioc()->registerExport<IAccessibilityConfiguration>(moduleName(), new AccessibilityConfiguration());
    ioc()->registerExport<IAccessibilityController>(moduleName(), s_accessibilityController);
}

void AccessibilityModule::resolveImports()
{
    auto ir = ioc()->resolve<ui::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("musescore://devtools/accessible/tree"), "MuseScore/Accessibility/AccessibleDevDialog.qml");
    }

    auto ar = ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<AccessibilityUiActions>());
    }
}

void AccessibilityModule::registerResources()
{
    accessibility_init_qrc();
}

void AccessibilityModule::registerUiTypes()
{
    qmlRegisterType<AccessibleDevModel>("MuseScore.Accessibility", 1, 0, "AccessibleDevModel");
}

void AccessibilityModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (mode != framework::IApplication::RunMode::Editor) {
        return;
    }

    s_accessibilityController->init();
}
