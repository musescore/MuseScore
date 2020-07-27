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

#include "framework/global/globalmodule.h"
#include "framework/ui/uimodule.h"
#include "framework/uicomponents/uicomponentsmodule.h"
#include "framework/actions/actionsmodule.h"
#include "framework/shortcuts/shortcutsmodule.h"
#include "framework/workspace/workspacemodule.h"
#include "framework/network/networkmodule.h"
#include "framework/audio/audiomodule.h"
#include "framework/midi/midimodule.h"
#include "mu4/appshell/appshellmodule.h"
#include "mu4/cloud/cloudmodule.h"
#include "mu4/context/contextmodule.h"
#include "mu4/userscores/userscoresmodule.h"
#include "mu4/extensions/extensionsmodule.h"
#include "mu4/domain/notation/notationdomainmodule.h"
#include "mu4/domain/importexport/importexportmodule.h"
#include "mu4/scenes/common/commonscenemodule.h"
#include "mu4/scenes/notation/notationscenemodule.h"
#include "mu4/scenes/palette/palettemodule.h"
#include "mu4/domain/importexport/importexportmodule.h"
#include "mu4/scenes/inspector/inspectormodule.h"
#include "mu4/scenes/playback/playbackmodule.h"

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
        << new mu::domain::notation::NotationDomainModule()
        << new mu::scene::common::CommonSceneModule()
        << new mu::scene::notation::NotationSceneModule()
        << new mu::scene::playback::PlaybackModule()
#endif

#ifdef BUILD_TELEMETRY_MODULE
        << new TelemetrySetup()
#endif
#ifdef AVSOMR
        << new Ms::Avs::AvsOmrSetup()
#endif
        << new mu::framework::GlobalModule()
        << new mu::framework::UiModule()
        << new mu::framework::UiComponentsModule()
        << new mu::framework::NetworkModule()
        << new mu::domain::importexport::ImportExportModule()
        << new mu::scene::inspector::InspectorModule()
        << new mu::scene::palette::PaletteModule()
    ;
}

//---------------------------------------------------------
//   setup
//---------------------------------------------------------

void ModulesSetup::setup()
{
    for (mu::framework::IModuleSetup* m : m_modulesSetupList) {
        m->registerExports();
    }

    for (mu::framework::IModuleSetup* m : m_modulesSetupList) {
        m->registerUiTypes();
        m->resolveImports();
        m->registerResources();
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
