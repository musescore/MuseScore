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

#include "consoleapp.h"

#include <QCoreApplication>

#include "modularity/ioc.h"
#include "audioplugins/iregisteraudiopluginsscenario.h"

#include "muse_framework_config.h"
#include "app_config.h"

#include "log.h"
#include "settings.h"

using namespace muse;
using namespace mu::app;
using namespace mu::converter;

static std::optional<ConvertTarget> parseTarget(const QMap<MuseScoreCmdOptions::ParamKey, QVariant>& params)
{
    auto it = params.find(MuseScoreCmdOptions::ParamKey::ScoreRegion);
    if (it != params.end()) {
        return it.value().toString().toStdString();
    }

    it = params.find(MuseScoreCmdOptions::ParamKey::PageNumber);
    if (it == params.end()) {
        return std::nullopt;
    }

    bool ok = true;
    page_num_t num = it.value().toULongLong(&ok) - 1;

    if (!ok) {
        LOGE() << "Invalid page, ignoring...";
        return std::nullopt;
    }

    return num;
}

MuseScoreConsoleApp::MuseScoreConsoleApp(const std::shared_ptr<MuseScoreCmdOptions>& options)
    : muse::ConsoleApplication(options)
{
}

void MuseScoreConsoleApp::showSplash()
{
    std::cout << "================================================" << std::endl;
    std::cout << "The MuseScore console application is starting..." << std::endl;
    std::cout << "================================================" << std::endl;
}

void MuseScoreConsoleApp::applyCommandLineOptions(const std::shared_ptr<muse::CmdOptions>& opt)
{
    IF_ASSERT_FAILED(opt) {
        return;
    }

    muse::ConsoleApplication::applyCommandLineOptions(opt);

    if (opt->runMode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    const std::shared_ptr<MuseScoreCmdOptions> options = std::dynamic_pointer_cast<MuseScoreCmdOptions>(opt);
    IF_ASSERT_FAILED(options) {
        return;
    }

    uiConfiguration()->setCustomPhysicalDotsPerInch(options->ui.physicalDotsPerInch);

    notationConfiguration()->setTemplateModeEnabled(options->notation.templateModeEnabled);
    notationConfiguration()->setTestModeEnabled(options->notation.testModeEnabled);

    if (options->runMode == IApplication::RunMode::ConsoleApp) {
        project::MigrationOptions migration;
        migration.appVersion = mu::engraving::Constants::MSC_VERSION;

        //! NOTE Don't ask about migration in convert mode
        migration.isAskAgain = false;

        if (options->project.fullMigration) {
            bool isMigration = options->project.fullMigration.value();
            migration.isApplyMigration = isMigration;
            migration.isApplyEdwin = isMigration;
            migration.isApplyLeland = isMigration;
            migration.isRemapPercussion = isMigration;
        }

        //! NOTE Don't write to settings, just on current session
        for (project::MigrationType type : project::allMigrationTypes()) {
            projectConfiguration()->setMigrationOptions(type, migration, false);
        }
    }

#ifdef MUE_BUILD_IMPEXP_IMAGESEXPORT_MODULE
    imagesExportConfiguration()->setTrimMarginPixelSize(options->exportImage.trimMarginPixelSize);
    imagesExportConfiguration()->setExportPngDpiResolutionOverride(options->exportImage.pngDpiResolution);
#endif

#ifdef MUE_BUILD_IMPEXP_VIDEOEXPORT_MODULE
    videoExportConfiguration()->setResolution(options->exportVideo.resolution);
    videoExportConfiguration()->setFps(options->exportVideo.fps);
    videoExportConfiguration()->setLeadingSec(options->exportVideo.leadingSec);
    videoExportConfiguration()->setTrailingSec(options->exportVideo.trailingSec);
#endif

#ifdef MUE_BUILD_IMPEXP_MIDI_MODULE
    midiImportExportConfiguration()->setMidiImportOperationsFile(options->importMidi.operationsFile);
#endif
#ifdef MUE_BUILD_IMPEXP_MUSICXML_MODULE
    musicXmlConfiguration()->setNeedUseDefaultFontOverride(options->importMusicXml.useDefaultFont);
    musicXmlConfiguration()->setInferTextTypeOverride(options->importMusicXml.inferTextType);
#endif
#ifdef MUE_BUILD_IMPEXP_AUDIOEXPORT_MODULE
    audioExportConfiguration()->setExportMp3BitrateOverride(options->exportAudio.mp3Bitrate);
#endif
#ifdef MUE_BUILD_IMPEXP_GUITARPRO_MODULE
    guitarProConfiguration()->setLinkedTabStaffCreated(options->guitarPro.linkedTabStaffCreated);
    guitarProConfiguration()->setExperimental(options->guitarPro.experimental);
#endif
    if (options->app.revertToFactorySettings) {
        settings()->reset(options->app.revertToFactorySettings.value());
    }
}

void MuseScoreConsoleApp::doStartupScenario(const muse::modularity::ContextPtr& ctxId)
{
    const std::shared_ptr<MuseScoreCmdOptions> options = std::dynamic_pointer_cast<MuseScoreCmdOptions>(contextData(ctxId).options);
    IF_ASSERT_FAILED(options) {
        return;
    }

    switch (options->runMode) {
    case IApplication::RunMode::ConsoleApp: {
        // ====================================================
        // Process Autobot
        // ====================================================
        MuseScoreCmdOptions::Autobot autobotOptions = options->autobot;
        if (!autobotOptions.testCaseNameOrFile.isEmpty()) {
            processAutobot(autobotOptions, ctxId);
        } else {
            // ====================================================
            // Process Diagnostic
            // ====================================================
            MuseScoreCmdOptions::Diagnostic diagnostic = options->diagnostic;
            if (diagnostic.type != DiagnosticType::Undefined) {
                int code = processDiagnostic(diagnostic, ctxId);
                qApp->exit(code);
            } else {
                // ====================================================
                // Process Converter
                // ====================================================
                MuseScoreCmdOptions::ConverterTask task = options->converterTask;
                int code = processConverter(task, ctxId);
                qApp->exit(code);
            }
        }
    } break;
    case IApplication::RunMode::AudioPluginRegistration: {
        MuseScoreCmdOptions::AudioPluginRegistration pluginRegistration = options->audioPluginRegistration;

        int code = processAudioPluginRegistration(pluginRegistration, ctxId);
        qApp->exit(code);
    } break;
    default: {
        UNREACHABLE;
    }
    }
}

int MuseScoreConsoleApp::processConverter(const MuseScoreCmdOptions::ConverterTask& task, const muse::modularity::ContextPtr& ctx)
{
    muse::ContextInject<converter::IConverterController> converter = { ctx };
    muse::GlobalInject<playback::ISoundProfilesRepository> soundProfilesRepository;

    Ret ret = make_ret(Ret::Code::Ok);
    String soundProfile = task.params[MuseScoreCmdOptions::ParamKey::SoundProfile].toString();
    UriQuery extensionUri = UriQuery(task.params[MuseScoreCmdOptions::ParamKey::ExtensionUri].toString().toStdString());

    if (!soundProfile.isEmpty() && !soundProfilesRepository()->containsProfile(soundProfile)) {
        LOGE() << "Unknown sound profile: " << soundProfile;
        soundProfile.clear();
    }

    converter::OpenParams openParams;
    openParams.stylePath = task.params[MuseScoreCmdOptions::ParamKey::StylePath].toString();
    openParams.forceMode = task.params[MuseScoreCmdOptions::ParamKey::ForceMode].toBool();
    openParams.unrollRepeats = task.params[MuseScoreCmdOptions::ParamKey::UnrollRepeats].toBool();

    switch (task.type) {
    case ConvertType::Batch:
        ret = converter()->batchConvert(task.inputFile, openParams, soundProfile, extensionUri);
        break;
    case ConvertType::File: {
        std::string transposeOptionsJson = task.params[MuseScoreCmdOptions::ParamKey::ScoreTransposeOptions].toString().toStdString();
        std::optional<ConvertTarget> target = parseTarget(task.params);
        io::path_t tracksDiffPath = task.params[MuseScoreCmdOptions::ParamKey::TracksDiffPath].toString();
        ret = converter()->fileConvert(task.inputFile, task.outputFile, openParams, soundProfile, tracksDiffPath,
                                       extensionUri, transposeOptionsJson, target);
    } break;
    case ConvertType::ConvertScoreParts:
        ret = converter()->convertScoreParts(task.inputFile, task.outputFile, openParams);
        break;
    case ConvertType::ExportScoreMedia: {
        muse::io::path_t highlightConfigPath = task.params[MuseScoreCmdOptions::ParamKey::HighlightConfigPath].toString();
        ret = converter()->exportScoreMedia(task.inputFile, task.outputFile, openParams, highlightConfigPath);
    } break;
    case ConvertType::ExportScoreMeta:
        ret = converter()->exportScoreMeta(task.inputFile, task.outputFile, openParams);
        break;
    case ConvertType::ExportScoreParts:
        ret = converter()->exportScoreParts(task.inputFile, task.outputFile, openParams);
        break;
    case ConvertType::ExportScorePartsPdf:
        ret = converter()->exportScorePartsPdfs(task.inputFile, task.outputFile, openParams);
        break;
    case ConvertType::ExportScoreTranspose: {
        std::string scoreTranspose = task.params[MuseScoreCmdOptions::ParamKey::ScoreTransposeOptions].toString().toStdString();
        ret = converter()->exportScoreTranspose(task.inputFile, task.outputFile, scoreTranspose, openParams);
    } break;
    case ConvertType::ExportScoreElements: {
        ret = converter()->exportScoreElements(task.inputFile, task.outputFile, openParams);
    } break;
    case ConvertType::ExportScoreVideo: {
        ret = converter()->exportScoreVideo(task.inputFile, task.outputFile, openParams);
    } break;
    case ConvertType::SourceUpdate: {
        std::string scoreSource = task.params[MuseScoreCmdOptions::ParamKey::ScoreSource].toString().toStdString();
        ret = converter()->updateSource(task.inputFile, scoreSource, openParams.forceMode);
    } break;
    }

    if (!ret) {
        LOGE() << "failed convert, error: " << ret.toString();
    }

    return ret.code();
}

int MuseScoreConsoleApp::processDiagnostic(const MuseScoreCmdOptions::Diagnostic& task, const muse::modularity::ContextPtr&)
{
    if (!diagnosticDrawProvider()) {
        return make_ret(Ret::Code::NotSupported);
    }

    Ret ret = make_ret(Ret::Code::Ok);

    if (task.input.isEmpty()) {
        return make_ret(Ret::Code::UnknownError);
    }

    io::paths_t input;
    for (const QString& p : task.input) {
        input.push_back(p);
    }

    muse::io::path_t output = task.output;

    if (output.empty()) {
        output = "./";
    }

    switch (task.type) {
    case DiagnosticType::GenDrawData:
        ret = diagnosticDrawProvider()->generateDrawData(input.front(), output);
        break;
    case DiagnosticType::ComDrawData: {
        IF_ASSERT_FAILED(input.size() == 2) {
            return make_ret(Ret::Code::UnknownError);
        }
        ret = diagnosticDrawProvider()->compareDrawData(input.at(0), input.at(1), output);
    } break;
    case DiagnosticType::DrawDataToPng:
        ret = diagnosticDrawProvider()->drawDataToPng(input.front(), output);
        break;
    case DiagnosticType::DrawDiffToPng: {
        muse::io::path_t diffPath = input.at(0);
        muse::io::path_t refPath;
        if (input.size() > 1) {
            refPath = input.at(1);
        }
        ret = diagnosticDrawProvider()->drawDiffToPng(diffPath, refPath, output);
    } break;
    default:
        break;
    }

    if (!ret) {
        LOGE() << "diagnostic ret: " << ret.toString();
    }

    return ret.code();
}

int MuseScoreConsoleApp::processAudioPluginRegistration(const MuseScoreCmdOptions::AudioPluginRegistration& task,
                                                        const muse::modularity::ContextPtr& ctx)
{
    Ret ret = make_ret(Ret::Code::Ok);

    muse::ContextInject<audioplugins::IRegisterAudioPluginsScenario> registerAudioPluginsScenario = { ctx };

    if (task.failedPlugin) {
        ret = registerAudioPluginsScenario()->registerFailedPlugin(task.pluginPath, task.failCode);
    } else {
        ret = registerAudioPluginsScenario()->registerPlugin(task.pluginPath);
    }

    if (!ret) {
        LOGE() << ret.toString();
    }

    return ret.code();
}

void MuseScoreConsoleApp::processAutobot(const MuseScoreCmdOptions::Autobot& task, const muse::modularity::ContextPtr& ctx)
{
    using namespace muse::autobot;
    muse::ContextInject<IAutobot> autobot = { ctx };

    muse::async::Channel<StepInfo, Ret> stepCh = autobot()->stepStatusChanged();
    stepCh.onReceive(nullptr, [](const StepInfo& step, const Ret& ret) {
        if (!ret) {
            LOGE() << "failed step: " << step.name << ", ret: " << ret.toString();
            qApp->exit(ret.code());
        } else {
            LOGI() << "success step: " << step.name << ", ret: " << ret.toString();
        }
    });

    muse::async::Channel<muse::io::path_t, IAutobot::Status> statusCh = autobot()->statusChanged();
    statusCh.onReceive(nullptr, [](const muse::io::path_t& path, IAutobot::Status st) {
        if (st == IAutobot::Status::Finished) {
            LOGI() << "success finished, path: " << path;
            qApp->exit(0);
        }
    });

    IAutobot::Options opt;
    opt.context = task.testCaseContextNameOrFile;
    opt.contextVal = task.testCaseContextValue.toStdString();
    opt.func = task.testCaseFunc.toStdString();
    opt.funcArgs = task.testCaseFuncArgs.toStdString();

    autobot()->execScript(task.testCaseNameOrFile, opt);
}
