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

#include "global/internal/consoleapplication.h"
#include "../cmdoptions.h"

#include "global/globalmodule.h"

#include "modularity/imodulesetup.h"
#include "modularity/ioc.h"
#include "global/iapplication.h"
#include "converter/iconvertercontroller.h"
#include "engraving/devtools/drawdata/idiagnosticdrawprovider.h"
#include "autobot/iautobot.h"

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
class MuseScoreConsoleApp : public muse::ConsoleApplication
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
    muse::GlobalInject<engraving::IDiagnosticDrawProvider> diagnosticDrawProvider;

public:
    MuseScoreConsoleApp(const std::shared_ptr<MuseScoreCmdOptions>& options);

    void showSplash() override;

private:
    void applyCommandLineOptions(const std::shared_ptr<muse::CmdOptions>& options) override;
    void doStartupScenario(const muse::modularity::ContextPtr& ctxId) override;
    int processConverter(const MuseScoreCmdOptions::ConverterTask& task, const muse::modularity::ContextPtr& ctx);
    int processDiagnostic(const MuseScoreCmdOptions::Diagnostic& task, const muse::modularity::ContextPtr& ctx);
    int processAudioPluginRegistration(const MuseScoreCmdOptions::AudioPluginRegistration& task, const muse::modularity::ContextPtr& ctx);
    void processAutobot(const MuseScoreCmdOptions::Autobot& task, const muse::modularity::ContextPtr& ctx);
};
}

#endif // MU_CONSOLEAPP_APP_H
