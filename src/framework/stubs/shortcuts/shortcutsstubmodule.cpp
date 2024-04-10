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
#include "shortcutsstubmodule.h"

#include "modularity/ioc.h"

#include "shortcutsregisterstub.h"
#include "shortcutscontrollerstub.h"
#include "midiremotestub.h"
#include "shortcutsconfigurationstub.h"

#include "ui/iuiengine.h"

using namespace muse::shortcuts;
using namespace muse::modularity;

static void shortcuts_init_qrc()
{
    Q_INIT_RESOURCE(shortcuts);
}

std::string ShortcutsModule::moduleName() const
{
    return "shortcuts_stub";
}

void ShortcutsModule::registerExports()
{
    ioc()->registerExport<IShortcutsRegister>(moduleName(), new ShortcutsRegisterStub());
    ioc()->registerExport<IShortcutsController>(moduleName(), new ShortcutsControllerStub());
    ioc()->registerExport<IMidiRemote>(moduleName(), new MidiRemoteStub());
    ioc()->registerExport<IShortcutsConfiguration>(moduleName(), new ShortcutsConfigurationStub());
}

void ShortcutsModule::registerResources()
{
    shortcuts_init_qrc();
}

void ShortcutsModule::registerUiTypes()
{
}
