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
#include "view/mididevicemappingmodel.h"
#include "view/editmidimappingmodel.h"

#include "ui/iuiengine.h"

#include "diagnostics/idiagnosticspathsregister.h"

using namespace mu::shortcuts;
using namespace mu::framework;
using namespace mu::modularity;
using namespace mu::ui;

static std::shared_ptr<ShortcutsController> s_shortcutsController = std::make_shared<ShortcutsController>();
static std::shared_ptr<ShortcutsRegister> s_shortcutsRegister = std::make_shared<ShortcutsRegister>();
static std::shared_ptr<ShortcutsConfiguration> s_configuration = std::make_shared<ShortcutsConfiguration>();
static std::shared_ptr<MidiRemote> s_midiRemote = std::make_shared<MidiRemote>();

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
    ioc()->registerExport<IShortcutsController>(moduleName(), s_shortcutsController);
    ioc()->registerExport<IMidiRemote>(moduleName(), s_midiRemote);
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
    qmlRegisterType<MidiDeviceMappingModel>("MuseScore.Shortcuts", 1, 0, "MidiDeviceMappingModel");
    qmlRegisterType<EditMidiMappingModel>("MuseScore.Shortcuts", 1, 0, "EditMidiMappingModel");

    ioc()->resolve<IUiEngine>(moduleName())->addSourceImportPath(shortcuts_QML_IMPORT);
}

void ShortcutsModule::onInit(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::Converter) {
        return;
    }

    s_configuration->init();
    s_shortcutsController->init();
    s_shortcutsRegister->init();
    s_midiRemote->init();

    auto pr = ioc()->resolve<diagnostics::IDiagnosticsPathsRegister>(moduleName());
    if (pr) {
        pr->reg("shortcutsUserAppDataPath", s_configuration->shortcutsUserAppDataPath());
        pr->reg("shortcutsAppDataPath", s_configuration->shortcutsAppDataPath());
        pr->reg("midiMappingUserAppDataPath", s_configuration->midiMappingUserAppDataPath());
    }
}
