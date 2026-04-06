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
#include "shortcutsstubmodule.h"

#include "modularity/ioc.h"

#include "shortcutsregisterstub.h"
#include "shortcutscontrollerstub.h"
#include "shortcutsconfigurationstub.h"

using namespace muse::shortcuts;
using namespace muse::modularity;

static const std::string mname("shortcuts_stub");

std::string ShortcutsModule::moduleName() const
{
    return mname;
}

void ShortcutsModule::registerExports()
{
    globalIoc()->registerExport<IShortcutsConfiguration>(mname, new ShortcutsConfigurationStub());
}

IContextSetup* ShortcutsModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new ShortcutsContext(ctx);
}

void ShortcutsContext::registerExports()
{
    ioc()->registerExport<IShortcutsRegister>(mname, new ShortcutsRegisterStub());
    ioc()->registerExport<IShortcutsController>(mname, new ShortcutsControllerStub());
}
