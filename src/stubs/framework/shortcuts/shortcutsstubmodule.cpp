//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "shortcutsstubmodule.h"

#include "modularity/ioc.h"

#include "shortcutsregisterstub.h"
#include "shortcutscontrollerstub.h"
#include "midiremotestub.h"
#include "shortcutsconfigurationstub.h"

#include "ui/iuiengine.h"

using namespace mu::shortcuts;
using namespace mu::framework;

static void shortcuts_init_qrc()
{
    Q_INIT_RESOURCE(shortcuts);
}

std::string ShortcutsStubModule::moduleName() const
{
    return "shortcuts_stub";
}

void ShortcutsStubModule::registerExports()
{
    ioc()->registerExport<IShortcutsRegister>(moduleName(), new ShortcutsRegisterStub());
    ioc()->registerExport<IShortcutsController>(moduleName(), new ShortcutsControllerStub());
    ioc()->registerExport<IMidiRemote>(moduleName(), new MidiRemoteStub());
    ioc()->registerExport<IShortcutsConfiguration>(moduleName(), new ShortcutsConfigurationStub());
}

void ShortcutsStubModule::registerResources()
{
    shortcuts_init_qrc();
}

void ShortcutsStubModule::registerUiTypes()
{
    ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(shortcuts_QML_IMPORT);
}
