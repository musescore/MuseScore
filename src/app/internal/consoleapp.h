/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_CONSOLEAPP_APP_H
#define MU_CONSOLEAPP_APP_H

#include <vector>
#include <memory>

#include "global/internal/baseapplication.h"
#include "../cmdoptions.h"

#include "global/globalmodule.h"

#include "modularity/imodulesetup.h"
#include "modularity/ioc.h"
#include "global/iapplication.h"
#include "converter/iconvertercontroller.h"
#include "engraving/devtools/drawdata/idiagnosticdrawprovider.h"
#include "autobot/iautobot.h"
#include "audioplugins/iregisteraudiopluginsscenario.h"
#include "multiinstances/imultiinstancesprovider.h"

#include "ui/iuiconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "project/iprojectconfiguration.h"
#include "playback/isoundprofilesrepository.h"
#include "appshell/iappshellconfiguration.h"
#include "appshell/internal/istartupscenario.h"
#include "importexport/imagesexport/iimagesexportconfiguration.h"
#include "importexport/midi/imidiconfiguration.h"
#include "importexport/audioexport/iaudioexportconfiguration.h"
#include "importexport/videoexport/ivideoexportconfiguration.h"
#include "importexport/guitarpro/iguitarproconfiguration.h"
#include "importexport/musicxml/imusicxmlconfiguration.h"

namespace mu::app {
class ConsoleApp : public muse::BaseApplication, public std::enable_shared_from_this<ConsoleApp>
{
    muse::Inject<muse::IApplication> muapplication;
    muse::Inject<converter::IConverterController> converter;
    muse::Inject<engraving::IDiagnosticDrawProvider> diagnosticDrawProvider;
    muse::Inject<muse::autobot::IAutobot> autobot;
    muse::Inject<muse::audioplugins::IRegisterAudioPluginsScenario> registerAudioPluginsScenario;
    muse::Inject<muse::mi::IMultiInstancesProvider> multiInstancesProvider;
    muse::Inject<muse::ui::IUiConfiguration> uiConfiguration;
    muse::Inject<appshell::IAppShellConfiguration> appshellConfiguration;
    muse::Inject<appshell::IStartupScenario> startupScenario;
    muse::Inject<notation::INotationConfiguration> notationConfiguration;
    muse::Inject<project::IProjectConfiguration> projectConfiguration;
    muse::Inject<playback::ISoundProfilesRepository> soundProfilesRepository;
    muse::Inject<iex::imagesexport::IImagesExportConfiguration> imagesExportConfiguration;
    muse::Inject<iex::midi::IMidiImportExportConfiguration> midiImportExportConfiguration;
    muse::Inject<iex::audioexport::IAudioExportConfiguration> audioExportConfiguration;
    muse::Inject<iex::videoexport::IVideoExportConfiguration> videoExportConfiguration;
    muse::Inject<iex::guitarpro::IGuitarProConfiguration> guitarProConfiguration;
    muse::Inject<iex::musicxml::IMusicXmlConfiguration> musicXmlConfiguration;

public:
    ConsoleApp(const CmdOptions& options, const muse::modularity::ContextPtr& ctx);

    void addModule(muse::modularity::IModuleSetup* module);

    void perform() override;
    void finish() override;

private:
    void applyCommandLineOptions(const CmdOptions& options, muse::IApplication::RunMode runMode);
    int processConverter(const CmdOptions::ConverterTask& task);
    int processDiagnostic(const CmdOptions::Diagnostic& task);
    int processAudioPluginRegistration(const CmdOptions::AudioPluginRegistration& task);
    void processAutobot(const CmdOptions::Autobot& task);

    CmdOptions m_options;

    //! NOTE Separately to initialize logger and profiler as early as possible
    muse::GlobalModule m_globalModule;

    std::vector<muse::modularity::IModuleSetup*> m_modules;
};
}

#endif // MU_CONSOLEAPP_APP_H
