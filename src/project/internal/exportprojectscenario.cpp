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
#include "exportprojectscenario.h"

#include "global/io/file.h"
#include "global/io/fileinfo.h"

#include "translation.h"
#include "defer.h"
#include "log.h"

using namespace muse;
using namespace muse::io;
using namespace mu::project;
using namespace mu::notation;

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

RetVal<muse::io::path_t> ExportProjectScenario::askExportPath(const INotationPtrList& notations, const ExportType& exportType,
                                                              INotationWriter::UnitType unitType, muse::io::path_t defaultPath) const
{
    INotationProjectPtr project = context()->currentProject();

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

    if (defaultPath == "") {
        defaultPath = configuration()->defaultSavingFilePath(project, filenameAddition, exportType.suffixes.front().toStdString());
    }

    RetVal<muse::io::path_t> exportPath;
    exportPath.val = interactive()->selectSavingFile(muse::qtrc("project/export", "Export"), defaultPath,
                                                     exportType.filter(), isCreatingOnlyOneFile);
    exportPath.ret = !exportPath.val.empty();

    return exportPath;
}

bool ExportProjectScenario::exportScores(const notation::INotationPtrList& notations, const muse::io::path_t destinationPath,
                                         INotationWriter::UnitType unitType, bool openDestinationFolderOnExport) const
{
    std::string suffix = io::suffix(destinationPath);
    INotationWriterPtr writer = writers()->writer(suffix);

    if (!writer) {
        return false;
    }

    IF_ASSERT_FAILED(writer->supportsUnitType(unitType)) {
        return false;
    }

    // Make sure we do this in the same as we did it in `askExportPath`, so before initializing notations
    bool isCreatingOnlyOneFile = guessIsCreatingOnlyOneFile(notations, unitType);
    bool isExportingOnlyOneScore = notations.size() == 1;

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

    // Backup view modes
    std::vector<ViewMode> viewModes = this->viewModes(notations);
    setViewModes(notations, ViewMode::PAGE);

    Progress* writerProgress = writer->progress();
    size_t fileCount = exportFileCount(notations, unitType);
    size_t currentFileNum = 0;

    if (writerProgress) {
        showExportProgress(isAudioExport(suffix));
        m_exportProgress.start();

        writerProgress->progressChanged().onReceive(this, [this, &currentFileNum, fileCount](int64_t current, int64_t total,
                                                                                             const std::string& status) {
            m_exportProgress.progress(currentFileNum * total + current, fileCount * total, status);
        });

        m_exportProgress.canceled().onNotify(this, [writer]() {
            writer->abort();
        });
    }

    DEFER {
        // Restore view modes
        setViewModes(notations, viewModes);

        if (writerProgress) {
            m_exportProgress.finish(muse::make_ok());
            writerProgress->progressChanged().resetOnReceive(this);
            m_exportProgress.finished().resetOnReceive(this);
        }
    };

    // If isCreatingOnlyOneFile, the save dialog has already asked whether to replace
    // any existing files. If the user cancels, the filepath will be empty, so we would
    // not reach this point. But if we do, existing files should be overridden.
    m_fileConflictPolicy = isCreatingOnlyOneFile ? FileConflictPolicy::ReplaceAll : FileConflictPolicy::Undefined;

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

                auto exportFunction = [writer, notation, options](io::IODevice& destinationDevice) {
                        return writer->write(notation, destinationDevice, options);
                    };

                Ret ret = doExportLoop(definitivePath, exportFunction);
                if (ret.code() == static_cast<int>(Ret::Code::Cancel)) {
                    return false;
                }

                ++currentFileNum;
            }
        }
    } break;
    case INotationWriter::UnitType::PER_PART: {
        for (const INotationPtr& notation : notations) {
            muse::io::path_t definitivePath = isCreatingOnlyOneFile
                                              ? destinationPath
                                              : completeExportPath(destinationPath, notation, isMainNotation(
                                                                       notation), isExportingOnlyOneScore);

            auto exportFunction = [writer, notation, options](IODevice& destinationDevice) {
                    return writer->write(notation, destinationDevice, options);
                };

            Ret ret = doExportLoop(definitivePath, exportFunction);
            if (ret.code() == static_cast<int>(Ret::Code::Cancel)) {
                return false;
            }

            ++currentFileNum;
        }
    } break;
    case INotationWriter::UnitType::MULTI_PART: {
        auto exportFunction = [writer, notations, options](IODevice& destinationDevice) {
                return writer->writeList(notations, destinationDevice, options);
            };

        Ret ret = doExportLoop(destinationPath, exportFunction);
        if (ret.code() == static_cast<int>(Ret::Code::Cancel)) {
            return false;
        }
    } break;
    }

    if (openDestinationFolderOnExport) {
        openFolder(destinationPath);
    }

    return true;
}

const ExportInfo& ExportProjectScenario::exportInfo() const
{
    return m_exportInfo;
}

void ExportProjectScenario::setExportInfo(const ExportInfo& exportInfo)
{
    if (m_exportInfo == exportInfo) {
        return;
    }

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

size_t ExportProjectScenario::exportFileCount(const INotationPtrList& notations, INotationWriter::UnitType unitType) const
{
    switch (unitType) {
    case INotationWriter::UnitType::PER_PAGE: {
        size_t count = 0;

        for (const INotationPtr& notation : notations) {
            count += notation->elements()->pages().size();
        }

        return count;
    };
    case INotationWriter::UnitType::PER_PART:
        return notations.size();
    case INotationWriter::UnitType::MULTI_PART:
        return 1;
    }

    return 0;
}

bool ExportProjectScenario::isMainNotation(INotationPtr notation) const
{
    return masterNotation()->notation() == notation;
}

IMasterNotationPtr ExportProjectScenario::masterNotation() const
{
    return context()->currentMasterNotation();
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

bool ExportProjectScenario::shouldReplaceFile(const QString& filename) const
{
    switch (m_fileConflictPolicy) {
    case FileConflictPolicy::ReplaceAll:
        return true;
    case FileConflictPolicy::SkipAll:
        return false;
    case FileConflictPolicy::Undefined: {
        constexpr int Replace = static_cast<int>(IInteractive::Button::CustomButton) + 1;
        constexpr int ReplaceAll = static_cast<int>(IInteractive::Button::CustomButton) + 2;
        constexpr int Skip = static_cast<int>(IInteractive::Button::CustomButton) + 3;
        constexpr int SkipAll = static_cast<int>(IInteractive::Button::CustomButton) + 4;

        IInteractive::Result result = interactive()->question(
            muse::trc("project/export", "File already exists"),
            muse::qtrc("project/export", "A file already exists with the filename %1. Do you want to replace it?")
            .arg(filename).toStdString(), {
                IInteractive::ButtonData(Replace, muse::trc("project/export", "Replace")),
                IInteractive::ButtonData(ReplaceAll, muse::trc("project/export", "Replace all")),
                IInteractive::ButtonData(Skip, muse::trc("project/export", "Skip")),
                IInteractive::ButtonData(SkipAll, muse::trc("project/export", "Skip all"))
            });

        switch (result.button()) {
        case ReplaceAll:
            m_fileConflictPolicy = FileConflictPolicy::ReplaceAll; // fallthrough
        case Replace:
            return true;
        case SkipAll:
            m_fileConflictPolicy = FileConflictPolicy::SkipAll; // fallthrough
        case Skip:
        default:
            return false;
        }
    } break;
    }

    return false;
}

bool ExportProjectScenario::askForRetry(const QString& filename) const
{
    IInteractive::Result result = interactive()->question(
        muse::trc("project/export", "Error"),
        muse::qtrc("project/export", "An error occurred while writing the file %1. Do you want to retry?")
        .arg(filename).toStdString(), { IInteractive::Button::Retry, IInteractive::Button::Abort });

    return result.standardButton() == IInteractive::Button::Retry;
}

Ret ExportProjectScenario::doExportLoop(const muse::io::path_t& scorePath, std::function<Ret(io::IODevice&)> exportFunction) const
{
    IF_ASSERT_FAILED(exportFunction) {
        return make_ret(Ret::Code::InternalError);
    }

    String filename = FileInfo(scorePath).fileName();
    if (fileSystem()->exists(scorePath) && !shouldReplaceFile(filename)) {
        return make_ret(Ret::Code::InternalError);
    }

    while (true) {
        io::File outputFile(scorePath);
        outputFile.setMeta("file_path", scorePath.toStdString());
        if (!outputFile.open(File::WriteOnly)) {
            if (askForRetry(filename)) {
                continue;
            } else {
                return make_ret(Ret::Code::Cancel);
            }
        }

        Ret ret = exportFunction(outputFile);
        outputFile.close();

        if (!ret) {
            if (ret.code() == static_cast<int>(Ret::Code::Cancel)) {
                fileSystem()->remove(scorePath);
                return ret;
            }

            if (askForRetry(filename)) {
                continue;
            } else {
                return make_ret(Ret::Code::Cancel);
            }
        }

        break;
    }

    return muse::make_ok();
}

void ExportProjectScenario::showExportProgress(bool isAudioExport) const
{
    std::string title = isAudioExport ? muse::trc("project/export", "Exporting audio…") : muse::trc("project/export", "Exporting…");

    Ret ret = interactive()->showProgress(title, &m_exportProgress);
    if (!ret) {
        LOGE() << ret.toString();
    }
}

void ExportProjectScenario::openFolder(const muse::io::path_t& path) const
{
    Ret ret = interactive()->revealInFileBrowser(path.toQString());

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
