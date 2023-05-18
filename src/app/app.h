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

#ifndef MU_APP_APP_H
#define MU_APP_APP_H

#include <QList>

#include "modularity/imodulesetup.h"
#include "modularity/ioc.h"
#include "global/iapplication.h"
#include "converter/iconvertercontroller.h"
#include "diagnostics/idiagnosticdrawprovider.h"
#include "autobot/iautobot.h"
#include "audio/iregisteraudiopluginsscenario.h"

#include "ui/iuiconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "project/iprojectconfiguration.h"
#include "appshell/iappshellconfiguration.h"
#include "appshell/internal/istartupscenario.h"
#include "importexport/imagesexport/iimagesexportconfiguration.h"
#include "importexport/midi/imidiconfiguration.h"
#include "importexport/audioexport/iaudioexportconfiguration.h"
#include "importexport/videoexport/ivideoexportconfiguration.h"
#include "importexport/guitarpro/iguitarproconfiguration.h"

#include "commandlineparser.h"

namespace mu::app {
class App
{
    INJECT(framework::IApplication, muapplication)
    INJECT(converter::IConverterController, converter)
    INJECT(diagnostics::IDiagnosticDrawProvider, diagnosticDrawProvider)
    INJECT(autobot::IAutobot, autobot)
    INJECT(audio::IRegisterAudioPluginsScenario, registerAudioPluginsScenario)
    INJECT(ui::IUiConfiguration, uiConfiguration)
    INJECT(appshell::IAppShellConfiguration, appshellConfiguration)
    INJECT(appshell::IStartupScenario, startupScenario)
    INJECT(notation::INotationConfiguration, notationConfiguration)
    INJECT(project::IProjectConfiguration, projectConfiguration)
    INJECT(iex::imagesexport::IImagesExportConfiguration, imagesExportConfiguration)
    INJECT(iex::midi::IMidiImportExportConfiguration, midiImportExportConfiguration)
    INJECT(iex::audioexport::IAudioExportConfiguration, audioExportConfiguration)
    INJECT(iex::videoexport::IVideoExportConfiguration, videoExportConfiguration)
    INJECT(iex::guitarpro::IGuitarProConfiguration, guitarProConfiguration)

public:
    App();

    void addModule(modularity::IModuleSetup* module);

    int run(int argc, char** argv);

private:
    void applyCommandLineOptions(const CommandLineParser::Options& options, framework::IApplication::RunMode runMode);
    int processConverter(const CommandLineParser::ConverterTask& task);
    int processDiagnostic(const CommandLineParser::Diagnostic& task);
    int processAudioPluginRegistration(const CommandLineParser::AudioPluginRegistration& task);
    void processAutobot(const CommandLineParser::Autobot& task);

    QList<modularity::IModuleSetup*> m_modules;
};
}

#endif // MU_APP_APP_H
