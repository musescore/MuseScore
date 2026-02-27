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
#include <map>

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

#include "ui/iuiconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "project/iprojectconfiguration.h"
#include "playback/isoundprofilesrepository.h"
#include "importexport/imagesexport/iimagesexportconfiguration.h"
#include "importexport/midi/imidiconfiguration.h"
#include "importexport/audioexport/iaudioexportconfiguration.h"
#include "importexport/videoexport/ivideoexportconfiguration.h"
#include "importexport/guitarpro/iguitarproconfiguration.h"
#include "importexport/musicxml/imusicxmlconfiguration.h"

namespace mu::app {
class ConsoleApp : public muse::BaseApplication, public std::enable_shared_from_this<ConsoleApp>
{
    muse::GlobalInject<muse::ui::IUiConfiguration> uiConfiguration;
    muse::GlobalInject<notation::INotationConfiguration> notationConfiguration;
    muse::GlobalInject<project::IProjectConfiguration> projectConfiguration;
    muse::GlobalInject<iex::imagesexport::IImagesExportConfiguration> imagesExportConfiguration;
    muse::GlobalInject<iex::midi::IMidiImportExportConfiguration> midiImportExportConfiguration;
    muse::GlobalInject<iex::audioexport::IAudioExportConfiguration> audioExportConfiguration;
    muse::GlobalInject<iex::videoexport::IVideoExportConfiguration> videoExportConfiguration;
    muse::GlobalInject<iex::guitarpro::IGuitarProConfiguration> guitarProConfiguration;
    muse::GlobalInject<iex::musicxml::IMusicXmlConfiguration> musicXmlConfiguration;
    muse::GlobalInject<muse::audioplugins::IRegisterAudioPluginsScenario> registerAudioPluginsScenario;
    muse::GlobalInject<converter::IConverterController> converter;
    muse::GlobalInject<engraving::IDiagnosticDrawProvider> diagnosticDrawProvider;
    muse::ContextInject<muse::autobot::IAutobot> autobot = { this };
    muse::ContextInject<playback::ISoundProfilesRepository> soundProfilesRepository = { this };

public:
    ConsoleApp(const CmdOptions& options, const muse::modularity::ContextPtr& ctx);

    void addModule(muse::modularity::IModuleSetup* module);

    void setup() override;
    void finish() override;

    muse::modularity::ContextPtr setupNewContext(const muse::StringList& args = {}) override;
    void destroyContext(const muse::modularity::ContextPtr& ctx) override;
    int contextCount() const override;
    std::vector<muse::modularity::ContextPtr> contexts() const override;

private:
    void applyCommandLineOptions(const CmdOptions& options, muse::IApplication::RunMode runMode);
    int processConverter(const CmdOptions::ConverterTask& task);
    int processDiagnostic(const CmdOptions::Diagnostic& task);
    int processAudioPluginRegistration(const CmdOptions::AudioPluginRegistration& task);
    void processAutobot(const CmdOptions::Autobot& task);

    std::vector<muse::modularity::IContextSetup*>& contextSetups(const muse::modularity::ContextPtr& ctx);

    CmdOptions m_options;

    //! NOTE Separately to initialize logger and profiler as early as possible
    muse::GlobalModule* m_globalModule = nullptr;

    std::vector<muse::modularity::IModuleSetup*> m_modules;
    muse::modularity::ContextPtr m_context;

    struct Context {
        muse::modularity::ContextPtr ctx;
        std::vector<muse::modularity::IContextSetup*> setups;
    };

    std::vector<Context> m_contexts;
};
}

#endif // MU_CONSOLEAPP_APP_H
