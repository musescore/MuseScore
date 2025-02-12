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

#include <QApplication>
#ifndef Q_OS_WASM
#include <QThreadPool>
#endif

#include "modularity/ioc.h"

#include "muse_framework_config.h"

#include "log.h"

using namespace muse;
using namespace mu::app;
using namespace mu::appshell;

ConsoleApp::ConsoleApp(const CmdOptions& options, const modularity::ContextPtr& ctx)
    : muse::BaseApplication(ctx), m_options(options)
{
}

void ConsoleApp::addModule(modularity::IModuleSetup* module)
{
    m_modules.push_back(module);
}

void ConsoleApp::perform()
{
    const CmdOptions& options = m_options;

    IApplication::RunMode runMode = options.runMode;
    setRunMode(runMode);

    // ====================================================
    // Setup modules: Resources, Exports, Imports, UiTypes
    // ====================================================
    m_globalModule.setApplication(shared_from_this());
    m_globalModule.registerResources();
    m_globalModule.registerExports();
    m_globalModule.registerUiTypes();

    for (modularity::IModuleSetup* m : m_modules) {
        m->setApplication(shared_from_this());
        m->registerResources();
    }

    for (modularity::IModuleSetup* m : m_modules) {
        m->registerExports();
    }

    m_globalModule.resolveImports();
    m_globalModule.registerApi();
    for (modularity::IModuleSetup* m : m_modules) {
        m->registerUiTypes();
        m->resolveImports();
        m->registerApi();
    }

    // ====================================================
    // Setup modules: apply the command line options
    // ====================================================
    applyCommandLineOptions(options, runMode);

    // ====================================================
    // Setup modules: onPreInit
    // ====================================================
    m_globalModule.onPreInit(runMode);
    for (modularity::IModuleSetup* m : m_modules) {
        m->onPreInit(runMode);
    }

    // ====================================================
    // Setup modules: onInit
    // ====================================================
    m_globalModule.onInit(runMode);
    for (modularity::IModuleSetup* m : m_modules) {
        m->onInit(runMode);
    }

    // ====================================================
    // Setup modules: onAllInited
    // ====================================================
    m_globalModule.onAllInited(runMode);
    for (modularity::IModuleSetup* m : m_modules) {
        m->onAllInited(runMode);
    }

    // ====================================================
    // Setup modules: onStartApp (on next event loop)
    // ====================================================
    QMetaObject::invokeMethod(qApp, [this]() {
        m_globalModule.onStartApp();
        for (modularity::IModuleSetup* m : m_modules) {
            m->onStartApp();
        }
    }, Qt::QueuedConnection);

    // ====================================================
    // Run
    // ====================================================

    switch (runMode) {
    case IApplication::RunMode::ConsoleApp: {
        // ====================================================
        // Process Autobot
        // ====================================================
        CmdOptions::Autobot autobotOptions = options.autobot;
        if (!autobotOptions.testCaseNameOrFile.isEmpty()) {
            QMetaObject::invokeMethod(qApp, [this, autobotOptions]() {
                    processAutobot(autobotOptions);
                }, Qt::QueuedConnection);
        } else {
            // ====================================================
            // Process Diagnostic
            // ====================================================
            CmdOptions::Diagnostic diagnostic = options.diagnostic;
            if (diagnostic.type != DiagnosticType::Undefined) {
                QMetaObject::invokeMethod(qApp, [this, diagnostic]() {
                        int code = processDiagnostic(diagnostic);
                        qApp->exit(code);
                    }, Qt::QueuedConnection);
            } else {
                // ====================================================
                // Process Converter
                // ====================================================
                CmdOptions::ConverterTask task = options.converterTask;
                QMetaObject::invokeMethod(qApp, [this, task]() {
                        int code = processConverter(task);
                        qApp->exit(code);
                    }, Qt::QueuedConnection);
            }
        }
    } break;
    case IApplication::RunMode::AudioPluginRegistration: {
        CmdOptions::AudioPluginRegistration pluginRegistration = options.audioPluginRegistration;

        QMetaObject::invokeMethod(qApp, [this, pluginRegistration]() {
                int code = processAudioPluginRegistration(pluginRegistration);
                qApp->exit(code);
            }, Qt::QueuedConnection);
    } break;
    default: {
        UNREACHABLE;
    }
    }
}

void ConsoleApp::finish()
{
    PROFILER_PRINT;

// Wait Thread Poll
#ifndef Q_OS_WASM
    QThreadPool* globalThreadPool = QThreadPool::globalInstance();
    if (globalThreadPool) {
        LOGI() << "activeThreadCount: " << globalThreadPool->activeThreadCount();
        globalThreadPool->waitForDone();
    }
#endif

    // Deinit

    m_globalModule.invokeQueuedCalls();

    for (modularity::IModuleSetup* m : m_modules) {
        m->onDeinit();
    }

    m_globalModule.onDeinit();

    for (modularity::IModuleSetup* m : m_modules) {
        m->onDestroy();
    }

    m_globalModule.onDestroy();

    // Delete modules
    qDeleteAll(m_modules);
    m_modules.clear();

    removeIoC();
}

void ConsoleApp::applyCommandLineOptions(const CmdOptions& options, IApplication::RunMode runMode)
{
    uiConfiguration()->setPhysicalDotsPerInch(options.ui.physicalDotsPerInch);

    notationConfiguration()->setTemplateModeEnabled(options.notation.templateModeEnabled);
    notationConfiguration()->setTestModeEnabled(options.notation.testModeEnabled);

    if (runMode == IApplication::RunMode::ConsoleApp) {
        project::MigrationOptions migration;
        migration.appVersion = mu::engraving::Constants::MSC_VERSION;

        //! NOTE Don't ask about migration in convert mode
        migration.isAskAgain = false;

        if (options.project.fullMigration) {
            bool isMigration = options.project.fullMigration.value();
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

#ifdef MUE_BUILD_IMAGESEXPORT_MODULE
    imagesExportConfiguration()->setTrimMarginPixelSize(options.exportImage.trimMarginPixelSize);
    imagesExportConfiguration()->setExportPngDpiResolutionOverride(options.exportImage.pngDpiResolution);
#endif

#ifdef MUE_BUILD_VIDEOEXPORT_MODULE
    videoExportConfiguration()->setResolution(options.exportVideo.resolution);
    videoExportConfiguration()->setFps(options.exportVideo.fps);
    videoExportConfiguration()->setLeadingSec(options.exportVideo.leadingSec);
    videoExportConfiguration()->setTrailingSec(options.exportVideo.trailingSec);
#endif

#ifdef MUE_BUILD_IMPORTEXPORT_MODULE
    audioExportConfiguration()->setExportMp3BitrateOverride(options.exportAudio.mp3Bitrate);
    midiImportExportConfiguration()->setMidiImportOperationsFile(options.importMidi.operationsFile);
    guitarProConfiguration()->setLinkedTabStaffCreated(options.guitarPro.linkedTabStaffCreated);
    guitarProConfiguration()->setExperimental(options.guitarPro.experimental);
    musicXmlConfiguration()->setNeedUseDefaultFontOverride(options.importMusicXml.useDefaultFont);
    musicXmlConfiguration()->setInferTextTypeOverride(options.importMusicXml.inferTextType);
#endif

    if (options.app.revertToFactorySettings) {
        appshellConfiguration()->revertToFactorySettings(options.app.revertToFactorySettings.value());
    }

    if (options.app.loggerLevel) {
        m_globalModule.setLoggerLevel(options.app.loggerLevel.value());
    }
}

int ConsoleApp::processConverter(const CmdOptions::ConverterTask& task)
{
    Ret ret = make_ret(Ret::Code::Ok);
    muse::io::path_t stylePath = task.params[CmdOptions::ParamKey::StylePath].toString();
    bool forceMode = task.params[CmdOptions::ParamKey::ForceMode].toBool();
    String soundProfile = task.params[CmdOptions::ParamKey::SoundProfile].toString();
    UriQuery extensionUri = UriQuery(task.params[CmdOptions::ParamKey::ExtensionUri].toString().toStdString());

    if (!soundProfile.isEmpty() && !soundProfilesRepository()->containsProfile(soundProfile)) {
        LOGE() << "Unknown sound profile: " << soundProfile;
        soundProfile.clear();
    }

    switch (task.type) {
    case ConvertType::Batch:
        ret = converter()->batchConvert(task.inputFile, stylePath, forceMode, soundProfile, extensionUri);
        break;
    case ConvertType::File: {
        std::string transposeOptionsJson = task.params[CmdOptions::ParamKey::ScoreTransposeOptions].toString().toStdString();
        ret = converter()->fileConvert(task.inputFile, task.outputFile, stylePath, forceMode, soundProfile, extensionUri,
                                       transposeOptionsJson);
    } break;
    case ConvertType::ConvertScoreParts:
        ret = converter()->convertScoreParts(task.inputFile, task.outputFile, stylePath);
        break;
    case ConvertType::ExportScoreMedia: {
        muse::io::path_t highlightConfigPath = task.params[CmdOptions::ParamKey::HighlightConfigPath].toString();
        ret = converter()->exportScoreMedia(task.inputFile, task.outputFile, highlightConfigPath, stylePath, forceMode);
    } break;
    case ConvertType::ExportScoreMeta:
        ret = converter()->exportScoreMeta(task.inputFile, task.outputFile, stylePath, forceMode);
        break;
    case ConvertType::ExportScoreParts:
        ret = converter()->exportScoreParts(task.inputFile, task.outputFile, stylePath, forceMode);
        break;
    case ConvertType::ExportScorePartsPdf:
        ret = converter()->exportScorePartsPdfs(task.inputFile, task.outputFile, stylePath, forceMode);
        break;
    case ConvertType::ExportScoreTranspose: {
        std::string scoreTranspose = task.params[CmdOptions::ParamKey::ScoreTransposeOptions].toString().toStdString();
        ret = converter()->exportScoreTranspose(task.inputFile, task.outputFile, scoreTranspose, stylePath, forceMode);
    } break;
    case ConvertType::ExportScoreVideo: {
        ret = converter()->exportScoreVideo(task.inputFile, task.outputFile);
    } break;
    case ConvertType::SourceUpdate: {
        std::string scoreSource = task.params[CmdOptions::ParamKey::ScoreSource].toString().toStdString();
        ret = converter()->updateSource(task.inputFile, scoreSource, forceMode);
    } break;
    }

    if (!ret) {
        LOGE() << "failed convert, error: " << ret.toString();
    }

    return ret.code();
}

int ConsoleApp::processDiagnostic(const CmdOptions::Diagnostic& task)
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
    case DiagnosticType::ComDrawData:
        IF_ASSERT_FAILED(input.size() == 2) {
            return make_ret(Ret::Code::UnknownError);
        }
        ret = diagnosticDrawProvider()->compareDrawData(input.at(0), input.at(1), output);
        break;
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

int ConsoleApp::processAudioPluginRegistration(const CmdOptions::AudioPluginRegistration& task)
{
    Ret ret = make_ret(Ret::Code::Ok);

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

void ConsoleApp::processAutobot(const CmdOptions::Autobot& task)
{
    using namespace muse::autobot;
    muse::async::Channel<StepInfo, Ret> stepCh = autobot()->stepStatusChanged();
    stepCh.onReceive(nullptr, [](const StepInfo& step, const Ret& ret){
        if (!ret) {
            LOGE() << "failed step: " << step.name << ", ret: " << ret.toString();
            qApp->exit(ret.code());
        } else {
            LOGI() << "success step: " << step.name << ", ret: " << ret.toString();
        }
    });

    muse::async::Channel<muse::io::path_t, IAutobot::Status> statusCh = autobot()->statusChanged();
    statusCh.onReceive(nullptr, [](const muse::io::path_t& path, IAutobot::Status st){
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
