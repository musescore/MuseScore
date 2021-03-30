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
#include "internal/shortcutsconfiguration.h"
#include "view/shortcutsmodel.h"
#include "view/editshortcutmodel.h"

#include "ui/iuiengine.h"

using namespace mu::shortcuts;
using namespace mu::framework;
using namespace mu::ui;

static std::shared_ptr<ShortcutsRegister> s_shortcutsRegister = std::make_shared<ShortcutsRegister>();
static std::shared_ptr<ShortcutsConfiguration> s_configuration = std::make_shared<ShortcutsConfiguration>();

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
    ioc()->registerExport<IShortcutsRegister>(moduleName(), s_shortcutsRegister);
    ioc()->registerExport<IShortcutsController>(moduleName(), new ShortcutsController());
    ioc()->registerExport<IMidiRemote>(moduleName(), new MidiRemote());
    ioc()->registerExport<IShortcutsConfiguration>(moduleName(), s_configuration);
}

void ShortcutsModule::registerResources()
{
    shortcuts_init_qrc();
}

void ShortcutsModule::registerUiTypes()
{
    qmlRegisterType<ShortcutsInstanceModel>("MuseScore.Shortcuts", 1, 0, "ShortcutsInstanceModel");
    qmlRegisterType<ShortcutsModel>("MuseScore.Shortcuts", 1, 0, "ShortcutsModel");
    qmlRegisterType<EditShortcutModel>("MuseScore.Shortcuts", 1, 0, "EditShortcutModel");

    ioc()->resolve<IUiEngine>(moduleName())->addSourceImportPath(shortcuts_QML_IMPORT);
}

void ShortcutsModule::onInit(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::Converter) {
        return;
    }

    s_configuration->init();
    s_shortcutsRegister->load();
}
