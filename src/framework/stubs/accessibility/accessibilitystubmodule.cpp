/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited and others
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
#include "accessibilitystubmodule.h"

#include "modularity/ioc.h"

#include "accessibilitycontrollerstub.h"
#include "accessibleapprootobjectstub.h"

using namespace muse::accessibility;
using namespace muse::modularity;

static const std::string mname("accessibility_stub");

std::string AccessibilityModule::moduleName() const
{
    return mname;
}

void AccessibilityModule::registerExports()
{
    globalIoc()->registerExport<IAccessibleAppRootObject>(moduleName(), new AccessibleAppRootObjectStub());
}

IContextSetup* AccessibilityModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new AccessibilityContext(ctx);
}

void AccessibilityContext::registerExports()
{
    ioc()->registerExport<IAccessibilityController>(mname, new AccessibilityControllerStub());
}
