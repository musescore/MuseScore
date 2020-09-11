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

#include "framework/global/globalmodule.h"
#include "framework/ui/uimodule.h"
#include "framework/uicomponents/uicomponentsmodule.h"
#include "framework/fonts/fontsmodule.h"
#include "framework/actions/actionsmodule.h"
#include "framework/shortcuts/shortcutsmodule.h"
#include "framework/workspace/workspacemodule.h"
#include "framework/system/systemmodule.h"
#include "framework/network/networkmodule.h"
#include "framework/audio/audiomodule.h"
#include "framework/midi/midimodule.h"
#include "mu4/appshell/appshellmodule.h"
#include "mu4/cloud/cloudmodule.h"
#include "mu4/context/contextmodule.h"
#include "mu4/userscores/userscoresmodule.h"
#include "mu4/extensions/extensionsmodule.h"
#include "mu4/languages/languagesmodule.h"
#include "mu4/plugins/pluginsmodule.h"
#include "mu4/notation/notationmodule.h"
#include "mu4/importexport/importexportmodule.h"
#include "mu4/importexport/importexportmodule.h"
#include "mu4/commonscene/commonscenemodule.h"
#include "mu4/palette/palettemodule.h"
#include "mu4/inspector/inspectormodule.h"
#include "mu4/playback/playbackmodule.h"
#include "mu4/instruments/instrumentsmodule.h"

#ifdef BUILD_VST
#include "framework/vst/vstmodule.h"
#endif

#ifdef BUILD_TELEMETRY_MODULE
#include "framework/telemetry/telemetrysetup.h"
#endif

#ifdef AVSOMR
#include "avsomr/avsomrsetup.h"
#endif

//---------------------------------------------------------
//   ModulesSetup
//---------------------------------------------------------

ModulesSetup::ModulesSetup()
{
    m_modulesSetupList
        << new mu::framework::GlobalModule()
        << new mu::framework::UiModule()
        << new mu::framework::UiComponentsModule()
        << new mu::fonts::FontsModule()
        << new mu::framework::SystemModule()
        << new mu::framework::NetworkModule()
        << new mu::plugins::PluginsModule()

#ifdef BUILD_UI_MU4
        << new mu::actions::ActionsModule()
        << new mu::appshell::AppShellModule()
        << new mu::cloud::CloudModule()
        << new mu::context::ContextModule()
        << new mu::shortcuts::ShortcutsModule()
        << new mu::workspace::WorkspaceModule()
        << new mu::audio::AudioModule()
        << new mu::midi::MidiModule()
        << new mu::userscores::UserScoresModule()
        << new mu::extensions::ExtensionsModule()
        << new mu::languages::LanguagesModule()
        << new mu::notation::NotationModule()
        << new mu::commonscene::CommonSceneModule()
        << new mu::playback::PlaybackModule()
        << new mu::instruments::InstrumentsModule()
#ifdef BUILD_VST
        << new mu::vst::VSTModule()
#endif
#endif

#ifdef BUILD_TELEMETRY_MODULE
        << new TelemetrySetup()
#endif
#ifdef AVSOMR
        << new Ms::Avs::AvsOmrSetup()
#endif
        << new mu::importexport::ImportExportModule()
        << new mu::inspector::InspectorModule()
        << new mu::palette::PaletteModule()
    ;
}

//---------------------------------------------------------
//   setup
//---------------------------------------------------------

void ModulesSetup::setup()
{
    mu::runtime::mainThreadId(); //! NOTE Needs only call
    mu::runtime::setThreadName("main");

    for (mu::framework::IModuleSetup* m : m_modulesSetupList) {
        m->registerResources();
    }

    for (mu::framework::IModuleSetup* m : m_modulesSetupList) {
        m->registerExports();
    }

    for (mu::framework::IModuleSetup* m : m_modulesSetupList) {
        m->registerUiTypes();
        m->resolveImports();
    }

    for (mu::framework::IModuleSetup* m : m_modulesSetupList) {
        m->onInit();
    }

    //! NOTE Need to move to the place where the application finishes initializing
    for (mu::framework::IModuleSetup* m : m_modulesSetupList) {
        m->onStartApp();
    }
}

void ModulesSetup::deinit()
{
    for (mu::framework::IModuleSetup* m : m_modulesSetupList) {
        m->onDeinit();
    }
}
