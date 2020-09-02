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
#include "shortcutsmodule.h"

#include <QtQml>

#include "log.h"

#include "modularity/ioc.h"
#include "internal/shortcutsinstancemodel.h"
#include "internal/shortcutsregister.h"
#include "internal/shortcutscontroller.h"
#include "internal/midiremote.h"

#include "ui/iuiengine.h"

using namespace mu::shortcuts;

static ShortcutsRegister* m_shortcutsRegister = new ShortcutsRegister();

static void shortcuts_init_qrc()
{
    Q_INIT_RESOURCE(shortcuts);
}

std::string ShortcutsModule::moduleName() const
{
    return "shortcuts";
}

void ShortcutsModule::registerExports()
{
    framework::ioc()->registerExport<IShortcutsRegister>(moduleName(), m_shortcutsRegister);
    framework::ioc()->registerExport<IShortcutsController>(moduleName(), new ShortcutsController());
    framework::ioc()->registerExport<IMidiRemote>(moduleName(), new MidiRemote());
}

void ShortcutsModule::registerResources()
{
    shortcuts_init_qrc();
}

void ShortcutsModule::registerUiTypes()
{
    qmlRegisterType<ShortcutsInstanceModel>("MuseScore.Shortcuts", 1, 0, "ShortcutsInstanceModel");

    framework::ioc()->resolve<framework::IUiEngine>(moduleName())->addSourceImportPath(shortcuts_QML_IMPORT);
}

void ShortcutsModule::onInit()
{
    m_shortcutsRegister->load();
}
