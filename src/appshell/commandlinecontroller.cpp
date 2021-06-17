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

    // Converter mode
    m_parser.addOption(QCommandLineOption({ "r", "image-resolution" }, "Set output resolution for image export", "DPI"));
    m_parser.addOption(QCommandLineOption({ "j", "job" }, "Process a conversion job", "file"));
    m_parser.addOption(QCommandLineOption({ "o", "export-to" }, "Export to 'file'. Format depends on file's extension", "file"));
    m_parser.addOption(QCommandLineOption({ "F", "factory-settings" }, "Use factory settings"));
    m_parser.addOption(QCommandLineOption({ "R", "revert-settings" }, "Revert to factory settings, but keep default preferences"));

    m_parser.addOption(QCommandLineOption("score-media",
                                          "Export all media (excepting mp3) for a given score in a single JSON file and print it to stdout"));
    m_parser.addOption(QCommandLineOption("highlight-config", "Set highlight to svg, generated from a given score", "highlight-config"));
    m_parser.addOption(QCommandLineOption("score-meta", "Export score metadata to JSON document and print it to stdout"));
    m_parser.addOption(QCommandLineOption("score-parts", "Generate parts data for the given score and save them to separate mscz files"));

    m_parser.process(args);
}

void CommandLineController::apply()
{
    auto floatValue = [this](const QString& name) -> std::optional<float> {
        bool ok;
        float val = m_parser.value(name).toFloat(&ok);
        if (ok) {
            return val;
        }
        return std::nullopt;
    };

    // Common
    // TODO: Open these files at launch if RunMode is Editor
    QStringList scorefiles = m_parser.positionalArguments();

    if (m_parser.isSet("long-version")) {
        printLongVersion();
        exit(EXIT_SUCCESS);
    }

    if (m_parser.isSet("d")) {
        haw::logger::Logger::instance()->setLevel(haw::logger::Debug);
    }

    if (m_parser.isSet("D")) {
        std::optional<float> val = floatValue("D");
        if (val) {
            uiConfiguration()->setPhysicalDotsPerInch(val);
        } else {
            LOGE() << "Option: -D not recognized DPI value: " << m_parser.value("D");
        }
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
            m_converterTask.data = m_parser.value("highlight-config");
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

    if (m_parser.isSet("F") || m_parser.isSet("R")) {
        configuration()->revertToFactorySettings(m_parser.isSet("R"));
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
