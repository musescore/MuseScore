/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "exportprojectscenario.h"

#include <thread>

#include <QTimer>

#include "global/io/fileinfo.h"
#include "global/io/filestream.h"

#include "translation.h"
#include "log.h"

#include "engraving/dom/score.h"

#include "notation/iexcerptnotation.h" // IWYU pragma: keep
#include "notation/imasternotation.h"
#include "notation/inotation.h"
#include "notation/inotationelements.h" // IWYU pragma: keep
#include "notation/inotationpainting.h" // IWYU pragma: keep

#include "inotationproject.h"

using namespace muse;
using namespace muse::io;
using namespace mu::project;
using namespace mu::notation;
using muse::async::Promise;

static Promise<Ret> resolvedPromise(const Ret& ret)
{
    return async::make_promise<Ret>([ret](auto resolve) {
        return resolve(ret);
    });
}

struct ExportProjectScenario::ExportState {
    INotationPtrList notations;
    std::vector<ViewMode> viewModes;

    INotationWriterPtr writer;
    std::vector<ExportFile> files;
    size_t currentFileIndex = 0;
    bool isCurrentFileConfirmed = false; // the user agreed to replace or retry the current file
    FileConflictPolicy fileConflictPolicy = FileConflictPolicy::Undefined;

    muse::io::path_t destinationPath;
    bool openDestinationFolderOnExport = false;

    Progress* writerProgress = nullptr;
    bool isCanceled = false; // the user canceled the export, maybe between the files
    Promise<Ret>::Resolve resolve;

    void moveToNextFile()
    {
        ++currentFileIndex;
        isCurrentFileConfirmed = false;
    }
};

std::vector<INotationWriter::UnitType> ExportProjectScenario::supportedUnitTypes(const ExportType& exportType) const
{
    IF_ASSERT_FAILED(!exportType.suffixes.isEmpty()) {
        return {};
    }

    auto writer = writers()->writer(exportType.suffixes.front().toStdString());
    if (!writer) {
        return {};
    }

    return writer->supportedUnitTypes();
}

Promise<muse::io::path_t> ExportProjectScenario::askExportPath(const INotationPtrList& notations, const ExportType& exportType,
                                                               INotationWriter::UnitType unitType, muse::io::path_t defaultDirPath) const
{
    INotationProjectPtr project = context()->project();

    std::string filenameAddition;

    // If only one file will be created, the filename will be exactly what the user
    // types in the save dialog and therefore we can put the file dialog in charge of
    // asking the user whether an existing file should be overridden. Otherwise, we
    // will take care of that ourselves.
    bool isCreatingOnlyOneFile = guessIsCreatingOnlyOneFile(notations, unitType);
    bool isExportingOnlyOneScore = notations.size() == 1;

    if (unitType == INotationWriter::UnitType::MULTI_PART && !isExportingOnlyOneScore) {
        bool containsMaster = std::find_if(notations.cbegin(), notations.cend(), [this](INotationPtr notation) {
            return isMainNotation(notation);
        }) != notations.cend();

        if (containsMaster) {
            //: Used in export filename suggestion. Please use only characters that are valid for filenames.
            filenameAddition = "-" + muse::trc("project/export", "Score_and_Parts");
        } else {
            //: Used in export filename suggestion. Please use only characters that are valid for filenames.
            filenameAddition = "-" + muse::trc("project/export", "Parts");
        }
    } else if (isExportingOnlyOneScore) {
        if (!isMainNotation(notations.front())) {
            filenameAddition = "-" + io::escapeFileName(notations.front()->name()).toStdString();
        }

        if (unitType == INotationWriter::UnitType::PER_PAGE && isCreatingOnlyOneFile) {
            // So there is only one page
            filenameAddition += "-1";
        }
    }

    muse::io::path_t defaultPath;
    if (defaultDirPath == "") {
        defaultPath = configuration()->defaultSavingFilePath(project, filenameAddition, exportType.suffixes.front().toStdString());
    } else {
        defaultPath = defaultDirPath.appendingComponent(io::filename(project->path(), false) + filenameAddition)
                      .appendingSuffix(exportType.suffixes.front().toStdString());
    }

    return interactive()->selectSavingFile(muse::trc("project/export", "Export"), defaultPath, exportType.filter(), isCreatingOnlyOneFile);
}

Promise<Ret> ExportProjectScenario::exportScores(notation::INotationPtrList notations, const muse::io::path_t destinationPath,
                                                 INotationWriter::UnitType unitType, bool openDestinationFolderOnExport) const
{
    return async::make_promise<Ret>([this, notations, destinationPath, unitType, openDestinationFolderOnExport](auto resolve) {
        m_pendingExports.push_back([this, notations, destinationPath, unitType, openDestinationFolderOnExport, resolve]() {
            startExport(notations, destinationPath, unitType, openDestinationFolderOnExport, resolve);
        });

        //! NOTE The exports go one after another, so that they do not change the view modes of each other.
        //! The resolve is lost if it is called before the caller subscribes to the promise,
        //! so an export always starts on the next event loop
        if (!m_isExporting) {
            m_isExporting = true;
            startNextExportLater();
        }

        return Promise<Ret>::dummy_result();
    }, async::PromiseType::AsyncByBody);
}

void ExportProjectScenario::startNextExportLater() const
{
    QTimer::singleShot(0, &m_timersContext, [this]() {
        if (m_pendingExports.empty()) {
            m_isExporting = false;
            return;
        }

        std::function<void()> start = std::move(m_pendingExports.front());
        m_pendingExports.pop_front();
        start();
    });
}

void ExportProjectScenario::startExport(const notation::INotationPtrList& notations, const muse::io::path_t& destinationPath,
                                        INotationWriter::UnitType unitType, bool openDestinationFolderOnExport,
                                        const Promise<Ret>::Resolve& resolve) const
{
    std::string suffix = io::suffix(destinationPath);
    INotationWriterPtr writer = writers()->writer(suffix);

    if (!writer) {
        (void)resolve(make_ret(Ret::Code::NotSupported));
        startNextExportLater();
        return;
    }

    IF_ASSERT_FAILED(writer->supportsUnitType(unitType)) {
        (void)resolve(make_ret(Ret::Code::NotSupported));
        startNextExportLater();
        return;
    }

    // Make sure we do this in the same as we did it in `askExportPath`, so before initializing notations
    bool isCreatingOnlyOneFile = guessIsCreatingOnlyOneFile(notations, unitType);

    // The user might have selected not-yet-inited excerpts
    ExcerptNotationList excerptsToInit;
    ExcerptNotationList potentialExcerpts = masterNotation()->potentialExcerpts();

    for (const INotationPtr& notation : notations) {
        auto it = std::find_if(potentialExcerpts.cbegin(), potentialExcerpts.cend(), [notation](const IExcerptNotationPtr& excerpt) {
            return excerpt->notation() == notation;
        });

        if (it != potentialExcerpts.cend()) {
            excerptsToInit.push_back(*it);
        }
    }

    masterNotation()->initExcerpts(excerptsToInit);

    // Scores that are closed may have never been laid out, so we lay them out now
    for (const INotationPtr& notation : notations) {
        mu::engraving::Score* score = notation->elements()->msScore();
        if (!score->autoLayoutEnabled()) {
            score->doLayout();
        }
    }

    auto state = std::make_shared<ExportState>();
    state->notations = notations;
    state->destinationPath = destinationPath;
    state->openDestinationFolderOnExport = openDestinationFolderOnExport;

    // Backup view modes
    state->viewModes = viewModes(notations);
    setViewModes(notations, ViewMode::PAGE);

    state->writer = writer;
    state->files = makeExportFiles(notations, destinationPath, unitType, isCreatingOnlyOneFile);

    // If isCreatingOnlyOneFile, the save dialog has already asked whether to replace
    // any existing files. If the user cancels, the filepath will be empty, so we would
    // not reach this point. But if we do, existing files should be overridden.
    state->fileConflictPolicy = isCreatingOnlyOneFile ? FileConflictPolicy::ReplaceAll : FileConflictPolicy::Undefined;

    state->writerProgress = writer->progress();
    if (state->writerProgress) {
        showExportProgress(isAudioExport(suffix));
        m_exportProgress.start();

        state->writerProgress->progressChanged().onReceive(this, [this, state](int64_t current, int64_t total, const std::string& status) {
            m_exportProgress.progress(state->currentFileIndex * total + current, state->files.size() * total, status);
        }, async::Asyncable::Mode::SetReplace);

        m_exportProgress.canceled().onNotify(this, [writer, state]() {
            //! NOTE The writer aborts the file being written, but if there is none (e.g. a question is shown),
            //! the export has to know about the cancellation itself
            state->isCanceled = true;
            writer->abort();
        }, muse::async::Asyncable::Mode::SetReplace);
    }

    state->resolve = resolve;
    exportNextFiles(state);
}

const ExportInfo& ExportProjectScenario::exportInfo() const
{
    return m_exportInfo;
}

void ExportProjectScenario::setExportInfo(const ExportInfo& exportInfo)
{
    m_exportInfo = exportInfo;
}

bool ExportProjectScenario::guessIsCreatingOnlyOneFile(const notation::INotationPtrList& notations,
                                                       INotationWriter::UnitType unitType) const
{
    switch (unitType) {
    case INotationWriter::UnitType::PER_PAGE: {
        if (notations.size() == 1) {
            INotationPtr notation = notations.front();

            // Check if it is not a potential (not-yet-initialized) excerpt
            ExcerptNotationList potentialExcerpts = masterNotation()->potentialExcerpts();

            auto it = std::find_if(potentialExcerpts.cbegin(), potentialExcerpts.cend(), [notation](const IExcerptNotationPtr& excerpt) {
                    return excerpt->notation() == notation;
                });

            if (it == potentialExcerpts.cend()) {
                ViewMode viewMode = notation->painting()->viewMode();
                notation->painting()->setViewMode(ViewMode::PAGE);

                bool onePage = notations.front()->elements()->pages().size() == 1;

                notation->painting()->setViewMode(viewMode);

                return onePage;
            }
        }

        return false;
    };
    case INotationWriter::UnitType::PER_PART:
        return notations.size() == 1;
    case INotationWriter::UnitType::MULTI_PART:
        return true;
    }

    return false;
}

std::vector<ExportProjectScenario::ExportFile> ExportProjectScenario::makeExportFiles(const INotationPtrList& notations,
                                                                                      const muse::io::path_t& destinationPath,
                                                                                      INotationWriter::UnitType unitType,
                                                                                      bool isCreatingOnlyOneFile) const
{
    std::vector<ExportFile> files;

    bool isExportingOnlyOneScore = notations.size() == 1;

    INotationWriter::Options options {
        { INotationWriter::OptionKey::UNIT_TYPE, Val(unitType) },
    };

    switch (unitType) {
    case INotationWriter::UnitType::PER_PAGE: {
        for (const INotationPtr& notation : notations) {
            size_t pageCount = notation->elements()->msScore()->pages().size();
            bool isMain = isMainNotation(notation);

            for (size_t page = 0; page < pageCount; ++page) {
                options[INotationWriter::OptionKey::PAGE_NUMBER] = Val(static_cast<int>(page));

                muse::io::path_t definitivePath = isCreatingOnlyOneFile
                                                  ? destinationPath
                                                  : completeExportPath(destinationPath, notation, isMain, isExportingOnlyOneScore,
                                                                       static_cast<int>(page));

                files.push_back({ definitivePath, { notation }, false, options });
            }
        }
    } break;
    case INotationWriter::UnitType::PER_PART: {
        for (const INotationPtr& notation : notations) {
            muse::io::path_t definitivePath = isCreatingOnlyOneFile
                                              ? destinationPath
                                              : completeExportPath(destinationPath, notation, isMainNotation(
                                                                       notation), isExportingOnlyOneScore);

            files.push_back({ definitivePath, { notation }, false, options });
        }
    } break;
    case INotationWriter::UnitType::MULTI_PART: {
        files.push_back({ destinationPath, notations, true, options });
    } break;
    }

    return files;
}

bool ExportProjectScenario::isMainNotation(INotationPtr notation) const
{
    return masterNotation()->notation() == notation;
}

IMasterNotationPtr ExportProjectScenario::masterNotation() const
{
    return context()->masterNotation();
}

muse::io::path_t ExportProjectScenario::completeExportPath(const muse::io::path_t& basePath, INotationPtr notation, bool isMain,
                                                           bool isExportingOnlyOneScore, int pageIndex) const
{
    muse::io::path_t result = io::dirpath(basePath) + "/" + io::completeBasename(basePath);

    if (!isMain && !isExportingOnlyOneScore) {
        result += "-" + io::escapeFileName(notation->name()).toStdString();
    }

    if (pageIndex > -1) {
        result += "-" + std::to_string(pageIndex + 1);
    }

    result += "." + io::suffix(basePath);

    return result;
}

void ExportProjectScenario::exportNextFiles(const ExportStatePtr& state) const
{
    while (state->currentFileIndex < state->files.size()) {
        if (state->isCanceled) {
            finishExport(state, make_ret(Ret::Code::Cancel));
            return;
        }

        const ExportFile& file = state->files.at(state->currentFileIndex);

        if (!state->isCurrentFileConfirmed && fileSystem()->exists(file.path)) {
            if (state->fileConflictPolicy == FileConflictPolicy::Undefined) {
                askForReplaceFile(state);
                return;
            }

            if (state->fileConflictPolicy == FileConflictPolicy::SkipAll) {
                state->moveToNextFile();
                continue;
            }
        }

        writeFile(state->writer, file).onResolve(this, [this, state](const Ret& ret) {
            if (onFileWritten(state, ret)) {
                continueExportLater(state);
            }
        });

        return;
    }

    finishExport(state, muse::make_ok());
}

bool ExportProjectScenario::onFileWritten(const ExportStatePtr& state, const Ret& ret) const
{
    if (ret) {
        state->moveToNextFile();
        return true;
    }

    if (ret.code() == static_cast<int>(Ret::Code::Cancel)) {
        finishExport(state, ret);
        return false;
    }

    askForRetry(state);
    return false;
}

void ExportProjectScenario::finishExport(const ExportStatePtr& state, const Ret& ret) const
{
    // Restore view modes
    setViewModes(state->notations, state->viewModes);

    if (state->writerProgress) {
        m_exportProgress.finish(muse::make_ok());
        state->writerProgress->progressChanged().disconnect(this);
        m_exportProgress.canceled().disconnect(this);
    }

    if (ret && state->openDestinationFolderOnExport) {
        openFolder(state->destinationPath);
    }

    (void)state->resolve(ret);

    startNextExportLater();
}

void ExportProjectScenario::askForReplaceFile(const ExportStatePtr& state) const
{
    constexpr int Replace = static_cast<int>(IInteractive::Button::CustomButton) + 1;
    constexpr int ReplaceAll = static_cast<int>(IInteractive::Button::CustomButton) + 2;
    constexpr int Skip = static_cast<int>(IInteractive::Button::CustomButton) + 3;
    constexpr int SkipAll = static_cast<int>(IInteractive::Button::CustomButton) + 4;

    const String filename = FileInfo(state->files.at(state->currentFileIndex).path).fileName();

    interactive()->question(
        muse::trc("project/export", "File already exists"),
        muse::qtrc("project/export", "A file already exists with the filename %1. Do you want to replace it?")
        .arg(filename).toStdString(), {
        IInteractive::ButtonData(Replace, muse::trc("project/export", "Replace")),
        IInteractive::ButtonData(ReplaceAll, muse::trc("project/export", "Replace all")),
        IInteractive::ButtonData(Skip, muse::trc("project/export", "Skip")),
        IInteractive::ButtonData(SkipAll, muse::trc("project/export", "Skip all"))
    })
    .onResolve(this, [this, state](const IInteractive::Result& result) {
        switch (result.button()) {
            case ReplaceAll:
                state->fileConflictPolicy = FileConflictPolicy::ReplaceAll; // fallthrough
            case Replace:
                state->isCurrentFileConfirmed = true;
                break;
            case SkipAll:
                state->fileConflictPolicy = FileConflictPolicy::SkipAll; // fallthrough
            case Skip:
            default:
                state->moveToNextFile();
                break;
        }

        continueExportLater(state);
    });
}

void ExportProjectScenario::askForRetry(const ExportStatePtr& state) const
{
    const String filename = FileInfo(state->files.at(state->currentFileIndex).path).fileName();

    interactive()->question(
        muse::trc("project/export", "Error"),
        muse::qtrc("project/export", "An error occurred while writing the file %1. Do you want to retry?")
        .arg(filename).toStdString(), { IInteractive::Button::Retry, IInteractive::Button::Abort })
    .onResolve(this, [this, state](const IInteractive::Result& result) {
        if (result.standardButton() != IInteractive::Button::Retry) {
            finishExport(state, make_ret(Ret::Code::Cancel));
            return;
        }

        state->isCurrentFileConfirmed = true;
        continueExportLater(state);
    });
}

void ExportProjectScenario::continueExportLater(const ExportStatePtr& state) const
{
    //! NOTE The writers may process events deep inside (e.g. the audio export),
    //! which can't be done neither in Async's callback nor while the question dialog is being closed,
    //! so continue the export on the next event loop
    QTimer::singleShot(0, &m_timersContext, [this, state]() {
        exportNextFiles(state);
    });
}

Promise<Ret> ExportProjectScenario::writeFile(const INotationWriterPtr& writer, const ExportFile& file) const
{
    //! NOTE Most writers stream to the destination device (file on disk).
    //! But there is one atypical case:
    //! Export score to unpacked directory - creates a directory and writes files to it

    // Remove previous file because we need to know whether
    // the writer writes at path
    Ret ret = fileSystem()->remove(file.path);
    if (!ret) {
        return resolvedPromise(ret);
    }

    //! NOTE The device must exist until the writing is finished
    auto outputFile = std::make_shared<FileStream>(file.path);
    outputFile->setMeta("file_path", file.path.toStdString());
    if (!outputFile->open(IODevice::WriteOnly)) {
        return resolvedPromise(make_ret(Ret::Code::UnknownError));
    }

    //! NOTE The multi-part export is synchronous
    Promise<Ret> writing = file.isMultiPart
                           ? resolvedPromise(writer->writeListSync(file.notations, *outputFile, file.options))
                           : writer->write(file.notations.front(), *outputFile, file.options);

    return async::make_promise<Ret>([this, writing, outputFile, path = file.path](auto resolve) mutable {
        writing.onResolve(this, [this, outputFile, path, resolve](const Ret& exportRet) {
            outputFile->close();

            if (exportRet.code() == static_cast<int>(Ret::Code::Cancel) && fileSystem()->exists(path)) {
                // On Windows, remove() may fail immediately after close()
                // because the file lock has not been released yet
                for (int i = 0; i < 10; ++i) {
                    if (fileSystem()->remove(path)) {
                        break;
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }

            (void)resolve(exportRet);
        });

        return Promise<Ret>::Result::unchecked();
    }, async::PromiseType::AsyncByBody);
}

void ExportProjectScenario::showExportProgress(bool isAudioExport) const
{
    std::string title = isAudioExport ? muse::trc("project/export", "Exporting audio…") : muse::trc("project/export", "Exporting…");

    interactive()->showProgress(title, m_exportProgress);
}

void ExportProjectScenario::openFolder(const muse::io::path_t& path) const
{
    Ret ret = platformInteractive()->revealInFileBrowser(path.toQString());

    if (!ret) {
        LOGE() << "Could not open folder: " << path.toQString();
    }
}

std::vector<ViewMode> ExportProjectScenario::viewModes(const notation::INotationPtrList& notations) const
{
    std::vector<ViewMode> modes;
    for (const INotationPtr& notation : notations) {
        modes.push_back(notation->painting()->viewMode());
    }

    return modes;
}

void ExportProjectScenario::setViewModes(const notation::INotationPtrList& notations,
                                         const std::vector<notation::ViewMode>& viewModes) const
{
    IF_ASSERT_FAILED(notations.size() == viewModes.size()) {
        return;
    }

    for (size_t i = 0; i < notations.size(); ++i) {
        notations[i]->painting()->setViewMode(viewModes[i]);
    }
}

void ExportProjectScenario::setViewModes(const notation::INotationPtrList& notations, ViewMode viewMode) const
{
    std::vector<notation::ViewMode> modes(notations.size(), viewMode);
    setViewModes(notations, modes);
}
