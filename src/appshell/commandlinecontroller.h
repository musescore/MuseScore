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
#ifndef MU_APPSHELL_COMMANDLINECONTROLLER_H
#define MU_APPSHELL_COMMANDLINECONTROLLER_H

#include <QCommandLineParser>
#include <QStringList>

#include "modularity/ioc.h"
#include "global/iapplication.h"
#include "ui/iuiconfiguration.h"
#include "importexport/imagesexport/iimagesexportconfiguration.h"
#include "importexport/midi/imidiconfiguration.h"
#include "importexport/audioexport/iaudioexportconfiguration.h"
#include "iappshellconfiguration.h"
#include "internal/istartupscenario.h"
#include "notation/inotationconfiguration.h"
#include "project/iprojectconfiguration.h"

namespace mu::appshell {
class CommandLineController
{
    INJECT(appshell, framework::IApplication, application)
    INJECT(appshell, ui::IUiConfiguration, uiConfiguration)
    INJECT(appshell, iex::imagesexport::IImagesExportConfiguration, imagesExportConfiguration)
    INJECT(appshell, iex::midi::IMidiImportExportConfiguration, midiImportExportConfiguration)
    INJECT(appshell, iex::audioexport::IAudioExportConfiguration, audioExportConfiguration)
    INJECT(appshell, IAppShellConfiguration, configuration)
    INJECT(appshell, IStartupScenario, startupScenario)
    INJECT(appshell, notation::INotationConfiguration, notationConfiguration)
    INJECT(appshell, project::IProjectConfiguration, projectConfiguration)

public:
    CommandLineController() = default;

    enum class ConvertType {
        File,
        Batch,
        ConvertScoreParts,
        ExportScoreMedia,
        ExportScoreMeta,
        ExportScoreParts,
        ExportScorePartsPdf,
        ExportScoreTranspose,
        SourceUpdate
    };

    enum class ParamKey {
        HighlightConfigPath,
        StylePath,
        ScoreSource,
        ScoreTransposeOptions,
        ForceMode
    };

    struct ConverterTask {
        ConvertType type = ConvertType::File;

        QString inputFile;
        QString outputFile;

        QMap<ParamKey, QVariant> params;
    };

    void parse(const QStringList& args);
    void apply();

    ConverterTask converterTask() const;

private:
    void printLongVersion() const;

    QCommandLineParser m_parser;
    ConverterTask m_converterTask;
};
}

#endif // MU_APPSHELL_COMMANDLINECONTROLLER_H
