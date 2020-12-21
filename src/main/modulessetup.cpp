//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA and others
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

#include "modulessetup.h"
#include "config.h"
#include "runtime.h"
#include "log.h"

#include "framework/global/globalmodule.h"
#include "framework/ui/uimodule.h"
#include "framework/uicomponents/uicomponentsmodule.h"
#include "framework/fonts/fontsmodule.h"
#include "framework/actions/actionsmodule.h"
#include "framework/shortcuts/shortcutsmodule.h"
#include "framework/system/systemmodule.h"
#include "framework/network/networkmodule.h"
#include "framework/audio/audiomodule.h"
#include "framework/midi/midimodule.h"

#include "appshell/appshellmodule.h"
#include "context/contextmodule.h"
#include "userscores/userscoresmodule.h"
#include "notation/notationmodule.h"
#include "commonscene/commonscenemodule.h"
#include "palette/palettemodule.h"
#include "inspector/inspectormodule.h"
#include "playback/playbackmodule.h"
#include "instruments/instrumentsmodule.h"

#ifdef BUILD_VST
#include "framework/vst/vstmodule.h"
#endif

#ifdef BUILD_TELEMETRY_MODULE
#include "framework/telemetry/telemetrysetup.h"
#endif

#ifndef Q_OS_WASM
#include "framework/workspace/workspacemodule.h"
#include "plugins/pluginsmodule.h"
#include "importexport/importexportmodule.h"
#include "cloud/cloudmodule.h"
#include "extensions/extensionsmodule.h"
#include "languages/languagesmodule.h"
#else
#include "wasmtest/wasmtestmodule.h"
#endif

//! NOTE Separately to initialize logger and profiler as early as possible
static mu::framework::GlobalModule globalModule;

ModulesSetup* ModulesSetup::instance()
{
    static ModulesSetup s;
    return &s;
}

ModulesSetup::ModulesSetup()
{
    //! NOTE `telemetry` must be first, because it install crash handler.
    //! others modules order not important (must be)

    m_modulesSetupList
#ifdef BUILD_TELEMETRY_MODULE
        << new mu::telemetry::TelemetrySetup()
#endif
        << new mu::framework::UiModule()
        << new mu::framework::UiComponentsModule()
        << new mu::fonts::FontsModule()
        << new mu::framework::SystemModule()
        << new mu::framework::NetworkModule()

        << new mu::actions::ActionsModule()
        << new mu::appshell::AppShellModule()
        << new mu::context::ContextModule()
        << new mu::shortcuts::ShortcutsModule()
        << new mu::audio::AudioModule()
        << new mu::midi::MidiModule()
        << new mu::userscores::UserScoresModule()
        << new mu::notation::NotationModule()
        << new mu::commonscene::CommonSceneModule()
        << new mu::playback::PlaybackModule()
        << new mu::instruments::InstrumentsModule()
#ifdef BUILD_VST
        << new mu::vst::VSTModule()
#endif
        << new mu::inspector::InspectorModule()
        << new mu::palette::PaletteModule()

#ifndef Q_OS_WASM
      //<< new mu::importexport::ImportExportModule()
        << new mu::workspace::WorkspaceModule()
        << new mu::plugins::PluginsModule()
        << new mu::cloud::CloudModule()
        << new mu::extensions::ExtensionsModule()
        << new mu::languages::LanguagesModule()
#else
        << new mu::wasmtest::WasmTestModule()
#endif
    ;
}

void ModulesSetup::setup()
{
    mu::runtime::mainThreadId(); //! NOTE Needs only call
    mu::runtime::setThreadName("main");

    globalModule.registerResources();
    globalModule.registerExports();
    globalModule.registerUiTypes();
    globalModule.onInit();

    //! NOTE Now we can use logger and profiler

    TRACEFUNC;

    for (mu::framework::IModuleSetup* m : m_modulesSetupList) {
        m->registerResources();
    }

    for (mu::framework::IModuleSetup* m : m_modulesSetupList) {
        m->registerExports();
    }

    globalModule.resolveImports();
    for (mu::framework::IModuleSetup* m : m_modulesSetupList) {
        m->registerUiTypes();
        m->resolveImports();
    }

    for (mu::framework::IModuleSetup* m : m_modulesSetupList) {
        m->onInit();
    }

    //! NOTE Need to move to the place where the application finishes initializing
    globalModule.onStartApp();
    for (mu::framework::IModuleSetup* m : m_modulesSetupList) {
        m->onStartApp();
    }
}

void ModulesSetup::deinit()
{
    for (mu::framework::IModuleSetup* m : m_modulesSetupList) {
        m->onDeinit();
    }

    PROFILER_PRINT;

    globalModule.onDeinit();
}
