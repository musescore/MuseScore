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
#include "commandlinecontroller.h"

#include "log.h"
#include "global/version.h"
#include "config.h"

using namespace mu::appshell;
using namespace mu::framework;

void CommandLineController::parse(const QStringList& args)
{
    // Common
    m_parser.addHelpOption(); // -?, -h, --help
    m_parser.addVersionOption(); // -v, --version

    m_parser.addPositionalArgument("scorefiles", "The files to open", "[scorefile...]");

    m_parser.addOption(QCommandLineOption("long-version", "Print detailed version information"));
    m_parser.addOption(QCommandLineOption({ "d", "debug" }, "Debug mode"));

    m_parser.addOption(QCommandLineOption({ "D", "monitor-resolution" }, "Specify monitor resolution", "DPI"));
    m_parser.addOption(QCommandLineOption({ "T", "trim-image" },
                                          "Use with '-o <file>.png' and '-o <file.svg>'. Trim exported image with specified margin (in pixels)",
                                          "margin"));

    m_parser.addOption(QCommandLineOption({ "b", "bitrate" }, "Use with '-o <file>.mp3', sets bitrate, in kbps", "bitrate"));

    m_parser.addOption(QCommandLineOption("template-mode", "Save template mode, no page size")); // and no platform and creationDate tags
    m_parser.addOption(QCommandLineOption({ "t", "test-mode" }, "Set test mode flag for all files")); // this includes --template-mode

    m_parser.addOption(QCommandLineOption("session-type", "Startup with given session type", "type")); // see StartupScenario::sessionTypeTromString

    // Converter mode
    m_parser.addOption(QCommandLineOption({ "r", "image-resolution" }, "Set output resolution for image export", "DPI"));
    m_parser.addOption(QCommandLineOption({ "j", "job" }, "Process a conversion job", "file"));
    m_parser.addOption(QCommandLineOption({ "o", "export-to" }, "Export to 'file'. Format depends on file's extension", "file"));
    m_parser.addOption(QCommandLineOption({ "F", "factory-settings" }, "Use factory settings"));
    m_parser.addOption(QCommandLineOption({ "R", "revert-settings" }, "Revert to factory settings, but keep default preferences"));
    m_parser.addOption(QCommandLineOption({ "M", "midi-operations" }, "Specify MIDI import operations file", "file"));
    m_parser.addOption(QCommandLineOption({ "P", "export-score-parts" }, "Use with '-o <file>.pdf', export score and parts"));
    m_parser.addOption(QCommandLineOption({ "f", "force" },
                                          "Use with '-o <file>', ignore warnings reg. score being corrupted or from wrong version"));

    m_parser.addOption(QCommandLineOption("score-media",
                                          "Export all media (excepting mp3) for a given score in a single JSON file and print it to stdout"));
    m_parser.addOption(QCommandLineOption("highlight-config", "Set highlight to svg, generated from a given score", "highlight-config"));
    m_parser.addOption(QCommandLineOption("score-meta", "Export score metadata to JSON document and print it to stdout"));
    m_parser.addOption(QCommandLineOption("score-parts", "Generate parts data for the given score and save them to separate mscz files"));
    m_parser.addOption(QCommandLineOption("score-parts-pdf",
                                          "Generate parts data for the given score and export the data to a single JSON file, print it to stdout"));
    m_parser.addOption(QCommandLineOption("score-transpose",
                                          "Transpose the given score and export the data to a single JSON file, print it to stdout",
                                          "options"));
    m_parser.addOption(QCommandLineOption("source-update", "Update the source in the given score"));

    m_parser.addOption(QCommandLineOption({ "S", "style" }, "Load style file", "style"));

    // Video export
    m_parser.addOption(QCommandLineOption("score-video", "Generate video for the given score and export it to file"));
// not implemented
//    m_parser.addOption(QCommandLineOption("view-mode",
//                                          "View mode [paged-float, paged-original, paged-float-height, pano, auto]. Auto (default) will choose the best mode according to number of instruments etc... Will show piano for piano score only",
//                                          "auto"));
// not implemented
//    m_parser.addOption(QCommandLineOption("piano", "Show Piano, works only if one part and not auto or float modes"));
//    m_parser.addOption(QCommandLineOption("piano-position", "Show Piano top or bottom. Default bottom", "bottom"));
    m_parser.addOption(QCommandLineOption("resolution", "Resolution [2160p, 1440p, 1080p, 720p, 480p, 360p]", "1080p"));
    m_parser.addOption(QCommandLineOption("fps", "Frame per second [60, 30, 24]", "24"));
    m_parser.addOption(QCommandLineOption("ls", "Pause before playback in seconds (3.0)", "3.0"));
    m_parser.addOption(QCommandLineOption("ts", "Pause before end of video in seconds (3.0)", "3.0"));

    m_parser.addOption(QCommandLineOption("gp-linked", "create tabulature linked staves for guitar pro"));
    m_parser.addOption(QCommandLineOption("gp-experimental", "experimental features for guitar pro import"));

    //! NOTE Currently only implemented `full` mode
    m_parser.addOption(QCommandLineOption("migration", "Whether to do migration with given mode, `full` - full migration", "mode"));

    m_parser.process(args);
}

void CommandLineController::apply()
{
    auto floatValue = [this](const QString& name) -> std::optional<float> {
        bool ok = true;
        float val = m_parser.value(name).toFloat(&ok);
        if (ok) {
            return val;
        }
        return std::nullopt;
    };

    auto doubleValue = [this](const QString& name) -> std::optional<double> {
        bool ok = true;
        double val = m_parser.value(name).toDouble(&ok);
        if (ok) {
            return val;
        }
        return std::nullopt;
    };

    auto intValue = [this](const QString& name) -> std::optional<int> {
        bool ok = true;
        int val = m_parser.value(name).toInt(&ok);
        if (ok) {
            return val;
        }
        return std::nullopt;
    };

    QStringList scorefiles = m_parser.positionalArguments();

    if (m_parser.isSet("long-version")) {
        printLongVersion();
        exit(EXIT_SUCCESS);
    }

    if (m_parser.isSet("d")) {
        haw::logger::Logger::instance()->setLevel(haw::logger::Debug);
    }

    if (m_parser.isSet("D")) {
        std::optional<double> val = doubleValue("D");
        if (val) {
            uiConfiguration()->setPhysicalDotsPerInch(val);
        } else {
            LOGE() << "Option: -D not recognized DPI value: " << m_parser.value("D");
        }
    }

    if (m_parser.isSet("T")) {
        std::optional<int> val = intValue("T");
        if (val) {
            imagesExportConfiguration()->setTrimMarginPixelSize(val);
        } else {
            LOGE() << "Option: -T not recognized trim value: " << m_parser.value("T");
        }
    }

    if (m_parser.isSet("M")) {
        midiImportExportConfiguration()->setMidiImportOperationsFile(m_parser.value("M").toStdString());
    }

    if (m_parser.isSet("b")) {
        std::optional<int> val = intValue("b");
        if (val) {
            audioExportConfiguration()->setExportMp3Bitrate(val);
        } else {
            LOGE() << "Option: -b not recognized bitrate value: " << m_parser.value("b");
        }
    }

    notationConfiguration()->setTemplateModeEnabled(m_parser.isSet("template-mode"));
    notationConfiguration()->setTestModeEnabled(m_parser.isSet("t"));

    QString modeType;
    if (m_parser.isSet("session-type")) {
        modeType = m_parser.value("session-type");
    }

    // Converter mode
    if (m_parser.isSet("r")) {
        std::optional<float> val = floatValue("r");
        if (val) {
            imagesExportConfiguration()->setExportPngDpiResolution(val);
        } else {
            LOGE() << "Option: -r not recognized DPI value: " << m_parser.value("r");
        }
    }

    if (m_parser.isSet("o")) {
        application()->setRunMode(IApplication::RunMode::Converter);
        m_converterTask.type = ConvertType::File;
        if (scorefiles.size() < 1) {
            LOGE() << "Option: -o no input file specified";
        } else {
            if (scorefiles.size() > 1) {
                LOGW() << "Option: -o multiple input files specified; processing only the first one";
            }
            m_converterTask.inputFile = scorefiles[0];
            m_converterTask.outputFile = m_parser.value("o");
        }
    }

    if (m_parser.isSet("P")) {
        if (m_converterTask.outputFile.isEmpty()) {
            LOGE() << "Option: -R no output file specified";
        } else {
            m_converterTask.type = ConvertType::ConvertScoreParts;
        }
    }

    if (m_parser.isSet("j")) {
        application()->setRunMode(IApplication::RunMode::Converter);
        m_converterTask.type = ConvertType::Batch;
        m_converterTask.inputFile = m_parser.value("j");
    }

    if (m_parser.isSet("score-media")) {
        application()->setRunMode(IApplication::RunMode::Converter);
        m_converterTask.type = ConvertType::ExportScoreMedia;
        m_converterTask.inputFile = scorefiles[0];
        if (m_parser.isSet("highlight-config")) {
            m_converterTask.params[CommandLineController::ParamKey::HighlightConfigPath] = m_parser.value("highlight-config");
        }
    }

    if (m_parser.isSet("score-meta")) {
        application()->setRunMode(IApplication::RunMode::Converter);
        m_converterTask.type = ConvertType::ExportScoreMeta;
        m_converterTask.inputFile = scorefiles[0];
    }

    if (m_parser.isSet("score-parts")) {
        application()->setRunMode(IApplication::RunMode::Converter);
        m_converterTask.type = ConvertType::ExportScoreParts;
        m_converterTask.inputFile = scorefiles[0];
    }

    if (m_parser.isSet("score-parts-pdf")) {
        application()->setRunMode(IApplication::RunMode::Converter);
        m_converterTask.type = ConvertType::ExportScorePartsPdf;
        m_converterTask.inputFile = scorefiles[0];
    }

    if (m_parser.isSet("score-transpose")) {
        application()->setRunMode(IApplication::RunMode::Converter);
        m_converterTask.type = ConvertType::ExportScoreTranspose;
        m_converterTask.inputFile = scorefiles[0];
        m_converterTask.params[CommandLineController::ParamKey::ScoreTransposeOptions] = m_parser.value("score-transpose");
    }

    if (m_parser.isSet("source-update")) {
        QStringList args = m_parser.positionalArguments();

        application()->setRunMode(IApplication::RunMode::Converter);
        m_converterTask.type = ConvertType::SourceUpdate;
        m_converterTask.inputFile = args[0];

        if (args.size() >= 2) {
            m_converterTask.params[CommandLineController::ParamKey::ScoreSource] = args[1];
        } else {
            LOGW() << "Option: --source-update no source specified";
        }
    }

    // Video
#ifdef BUILD_VIDEOEXPORT_MODULE
    if (m_parser.isSet("score-video")) {
        application()->setRunMode(IApplication::RunMode::Converter);
        m_converterTask.type = ConvertType::ExportScoreVideo;
        m_converterTask.inputFile = scorefiles[0];
        m_converterTask.outputFile = m_parser.value("o");

        using namespace mu::iex::videoexport;
// not implemented
//        if (m_parser.isSet("view-mode")) {
//            auto toViewMode = [](const QString& str) {
//                if ("auto" == str) {
//                    return ViewMode::Auto;
//                }
//                if ("paged-float" == str) {
//                    return ViewMode::PagedFloat;
//                }
//                if ("paged-original" == str) {
//                    return ViewMode::PagedOriginal;
//                }
//                if ("paged-float-height" == str) {
//                    return ViewMode::PagedFloatHeight;
//                }
//                if ("pano" == str) {
//                    return ViewMode::Pano;
//                }
//                return ViewMode::Auto;
//            };
//            videoExportConfiguration()->setViewMode(toViewMode(m_parser.value("view-mode")));
//        }

// not implemented
//        if (m_parser.isSet("piano")) {
//            videoExportConfiguration()->setShowPiano(true);
//        }

//        if (m_parser.isSet("piano-position")) {
//            auto toPianoPosition= [](const QString& str) {
//                if ("top" == str) {
//                    return PianoPosition::Top;
//                }
//                return PianoPosition::Bottom;
//            };
//            videoExportConfiguration()->setPianoPosition(toPianoPosition(m_parser.value("piano-position")));
//        }

        if (m_parser.isSet("resolution")) {
            videoExportConfiguration()->setResolution(m_parser.value("resolution").toStdString());
        }

        if (m_parser.isSet("fps")) {
            videoExportConfiguration()->setFps(intValue("fps"));
        }

        if (m_parser.isSet("ls")) {
            videoExportConfiguration()->setLeadingSec(doubleValue("ls"));
        }

        if (m_parser.isSet("ts")) {
            videoExportConfiguration()->setTrailingSec(doubleValue("ts"));
        }
    }
#endif

    if (m_parser.isSet("F") || m_parser.isSet("R")) {
        configuration()->revertToFactorySettings(m_parser.isSet("R"));
    }

    if (m_parser.isSet("f")) {
        m_converterTask.params[CommandLineController::ParamKey::ForceMode] = true;
    }

    if (m_parser.isSet("S")) {
        m_converterTask.params[CommandLineController::ParamKey::StylePath] = m_parser.value("S");
    }

    if (application()->runMode() == IApplication::RunMode::Converter) {
        project::MigrationOptions migration;
        migration.appVersion = mu::engraving::MSCVERSION;

        //! NOTE Don't ask about migration in convert mode
        migration.isAskAgain = false;

        if (m_parser.isSet("migration")) {
            QString val = m_parser.value("migration");
            bool isMigration = (val == "full") ? true : false;
            migration.isApplyMigration = isMigration;
            migration.isApplyEdwin = isMigration;
            migration.isApplyLeland = isMigration;
        }

        //! NOTE Don't write to settings, just on current session
        for (project::MigrationType type : project::allMigrationTypes()) {
            projectConfiguration()->setMigrationOptions(type, migration, false);
        }
    }

    if (application()->runMode() == IApplication::RunMode::Editor) {
        startupScenario()->setModeType(modeType);

        if (!scorefiles.isEmpty()) {
            startupScenario()->setStartupScorePath(scorefiles[0]);
        }
    }

    if (m_parser.isSet("gp-linked")) {
        guitarProConfiguration()->setLinkedTabStaffCreated(true);
    }

    if (m_parser.isSet("gp-experimental")) {
        guitarProConfiguration()->setExperimental(true);
    }
}

CommandLineController::ConverterTask CommandLineController::converterTask() const
{
    return m_converterTask;
}

void CommandLineController::printLongVersion() const
{
    if (Version::unstable()) {
        printf("MuseScore: Music Score Editor\nUnstable Prerelease for Version %s; Build %s\n",
               Version::version().c_str(), Version::revision().c_str());
    } else {
        printf("MuseScore: Music Score Editor; Version %s; Build %s\n",
               Version::version().c_str(), Version::revision().c_str());
    }
}
