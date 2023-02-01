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
#include <QFile>

#include "exportprojectscenario.h"

#include "async/async.h"

#include "translation.h"
#include "log.h"

using namespace mu::project;
using namespace mu::notation;
using namespace mu::framework;

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

mu::RetVal<mu::io::path_t> ExportProjectScenario::askExportPath(const INotationPtrList& notations, const ExportType& exportType,
                                                                INotationWriter::UnitType unitType) const
{
    INotationProjectPtr project = context()->currentProject();

    std::string filenameAddition;

    // If only one file will be created, the filename will be exactly what the user
    // types in the save dialog and therefore we can put the file dialog in charge of
    // asking the user whether an existing file should be overridden. Otherwise, we
    // will take care of that ourselves.
    bool isCreatingOnlyOneFile = this->isCreatingOnlyOneFile(notations, unitType);
    bool isExportingOnlyOneScore = notations.size() == 1;

    if (unitType == INotationWriter::UnitType::MULTI_PART && !isExportingOnlyOneScore) {
        bool containsMaster = std::find_if(notations.cbegin(), notations.cend(), [this](INotationPtr notation) {
            return isMainNotation(notation);
        }) != notations.cend();

        if (containsMaster) {
            //: Used in export filename suggestion. Please use only characters that are valid for filenames.
            filenameAddition = "-" + trc("project/export", "Score_and_Parts");
        } else {
            //: Used in export filename suggestion. Please use only characters that are valid for filenames.
            filenameAddition = "-" + trc("project/export", "Parts");
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

    io::path_t defaultPath = configuration()->defaultSavingFilePath(project, filenameAddition, exportType.suffixes.front().toStdString());

    RetVal<io::path_t> exportPath;
    exportPath.val = interactive()->selectSavingFile(qtrc("project/export", "Export"), defaultPath,
                                                     exportType.filter(), isCreatingOnlyOneFile);
    exportPath.ret = !exportPath.val.empty();

    return exportPath;
}

bool ExportProjectScenario::exportScores(const notation::INotationPtrList& notations, const io::path_t& destinationPath,
                                         INotationWriter::UnitType unitType, bool openDestinationFolderOnExport) const
{
    m_currentSuffix = io::suffix(destinationPath);
    m_currentWriter = writers()->writer(m_currentSuffix);

    if (!m_currentWriter) {
        return false;
    }

    IF_ASSERT_FAILED(m_currentWriter->supportsUnitType(unitType)) {
        return false;
    }

    // Scores that are closed may have never been laid out, so we lay them out now
    for (INotationPtr notation : notations) {
        mu::engraving::Score* score = notation->elements()->msScore();
        if (!score->isOpen()) {
            score->doLayout();
        }
    }

    bool isCreatingOnlyOneFile = this->isCreatingOnlyOneFile(notations, unitType);

    // If isCreatingOnlyOneFile, the save dialog has already asked whether to replace
    // any existing files. If the user cancels, the filepath will be empty, so we would
    // not reach this point. But if we do, existing files should be overridden.
    m_fileConflictPolicy = isCreatingOnlyOneFile ? FileConflictPolicy::ReplaceAll : FileConflictPolicy::Undefined;

    switch (unitType) {
    case INotationWriter::UnitType::PER_PAGE: {
        for (INotationPtr notation : notations) {
            for (size_t page = 0; page < notation->elements()->msScore()->pages().size(); page++) {
                INotationWriter::Options options {
                    { INotationWriter::OptionKey::UNIT_TYPE, Val(unitType) },
                    { INotationWriter::OptionKey::PAGE_NUMBER, Val(static_cast<int>(page)) },
                    { INotationWriter::OptionKey::TRANSPARENT_BACKGROUND,
                      Val(imagesExportConfiguration()->exportPngWithTransparentBackground()) }
                };

                io::path_t definitivePath = isCreatingOnlyOneFile
                                            ? destinationPath
                                            : completeExportPath(destinationPath, notation, isMainNotation(notation),
                                                                 static_cast<int>(page));

                auto exportFunction = [this, notation, options](QIODevice& destinationDevice) {
                        showExportProgressIfNeed();
                        return m_currentWriter->write(notation, destinationDevice, options);
                    };

                Ret ret = doExportLoop(definitivePath, exportFunction);
                if (ret.code() == static_cast<int>(Ret::Code::Cancel)) {
                    return false;
                }
            }
        }
    } break;
    case INotationWriter::UnitType::PER_PART: {
        for (INotationPtr notation : notations) {
            INotationWriter::Options options {
                { INotationWriter::OptionKey::UNIT_TYPE, Val(unitType) },
                { INotationWriter::OptionKey::TRANSPARENT_BACKGROUND,
                  Val(imagesExportConfiguration()->exportPngWithTransparentBackground()) }
            };

            io::path_t definitivePath = isCreatingOnlyOneFile
                                        ? destinationPath
                                        : completeExportPath(destinationPath, notation, isMainNotation(notation));

            auto exportFunction = [this, notation, options](QIODevice& destinationDevice) {
                    showExportProgressIfNeed();
                    return m_currentWriter->write(notation, destinationDevice, options);
                };

            Ret ret = doExportLoop(definitivePath, exportFunction);
            if (ret.code() == static_cast<int>(Ret::Code::Cancel)) {
                return false;
            }
        }
    } break;
    case INotationWriter::UnitType::MULTI_PART: {
        INotationWriter::Options options {
            { INotationWriter::OptionKey::UNIT_TYPE, Val(unitType) },
            { INotationWriter::OptionKey::TRANSPARENT_BACKGROUND, Val(imagesExportConfiguration()->exportPngWithTransparentBackground()) }
        };

        auto exportFunction = [this, notations, options](QIODevice& destinationDevice) {
                showExportProgressIfNeed();
                return m_currentWriter->writeList(notations, destinationDevice, options);
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

Progress ExportProjectScenario::progress() const
{
    return m_currentWriter ? m_currentWriter->progress() : Progress();
}

void ExportProjectScenario::abort()
{
    if (m_currentWriter) {
        m_currentWriter->abort();
    }
}

bool ExportProjectScenario::isCreatingOnlyOneFile(const INotationPtrList& notations, INotationWriter::UnitType unitType) const
{
    switch (unitType) {
    case INotationWriter::UnitType::PER_PAGE:
        return notations.size() == 1 && notations.front()->elements()->pages().size() == 1;
    case INotationWriter::UnitType::PER_PART:
        return notations.size() == 1;
    case INotationWriter::UnitType::MULTI_PART:
        return true;
    }

    return false;
}

bool ExportProjectScenario::isMainNotation(INotationPtr notation) const
{
    return context()->currentMasterNotation()->notation() == notation;
}

mu::io::path_t ExportProjectScenario::completeExportPath(const io::path_t& basePath, INotationPtr notation, bool isMain,
                                                         int pageIndex) const
{
    io::path_t result = io::dirpath(basePath) + "/" + io::basename(basePath);

    if (!isMain) {
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
            trc("project/export", "File already exists"),
            qtrc("project/export", "A file already exists with the filename %1. Do you want to replace it?")
            .arg(filename).toStdString(), {
                IInteractive::ButtonData(Replace, trc("project/export", "Replace")),
                IInteractive::ButtonData(ReplaceAll, trc("project/export", "Replace all")),
                IInteractive::ButtonData(Skip, trc("project/export", "Skip")),
                IInteractive::ButtonData(SkipAll, trc("project/export", "Skip all"))
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
        trc("project/export", "Error"),
        qtrc("project/export", "An error occurred while writing the file %1. Do you want to retry?")
        .arg(filename).toStdString(), { IInteractive::Button::Retry, IInteractive::Button::Abort });

    return result.standardButton() == IInteractive::Button::Retry;
}

mu::Ret ExportProjectScenario::doExportLoop(const io::path_t& scorePath, std::function<Ret(QIODevice&)> exportFunction) const
{
    IF_ASSERT_FAILED(exportFunction) {
        return make_ret(Ret::Code::InternalError);
    }

    QString filename = io::filename(scorePath).toQString();
    if (fileSystem()->exists(scorePath) && !shouldReplaceFile(filename)) {
        return make_ret(Ret::Code::InternalError);
    }

    while (true) {
        QFile outputFile(scorePath.toQString());
        if (!outputFile.open(QFile::WriteOnly)) {
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

    return make_ok();
}

void ExportProjectScenario::showExportProgressIfNeed() const
{
    if (m_currentWriter && m_currentWriter->supportsProgressNotifications()) {
        async::Async::call(this, [this]() {
            UriQuery query("musescore://project/export/progress");

            if (isAudioExport(m_currentSuffix)) {
                query.addParam("title", Val(trc("project/export", "Exporting audio…")));
            }

            interactive()->open(query);
        });
    }
}

void ExportProjectScenario::openFolder(const io::path_t& path) const
{
    Ret ret = interactive()->revealInFileBrowser(path.toQString());

    if (!ret) {
        LOGE() << "Could not open folder: " << path.toQString();
    }
}
