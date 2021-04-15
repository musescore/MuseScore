//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "exportscorescenario.h"

#include "translation.h"

using namespace mu::userscores;
using namespace mu::notation;
using namespace mu::framework;

std::vector<WriterUnitType> ExportScoreScenario::supportedUnitTypes(const ExportType& exportType) const
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

bool ExportScoreScenario::exportScores(const INotationPtrList& notations, const ExportType& exportType, WriterUnitType unitType) const
{
    IF_ASSERT_FAILED(!exportType.suffixes.isEmpty()) {
        return false;
    }

    /// If only one file will be created, the filename will be exactly what the user
    /// types in the save dialog and therefore we can put the file dialog in charge of
    /// asking the user whether an existing file should be overridden. Otherwise, we
    /// will take care of that ourselves.
    bool isCreatingOnlyOneFile = this->isCreatingOnlyOneFile(notations, unitType);
    bool isExportingOnlyOneScore = notations.size() == 1;

    IMasterNotationPtr currentMasterNotation = context()->currentMasterNotation();

    io::path suggestedPath = configuration()->scoresPath().val;
    io::path masterNotationDirPath = io::dirpath(currentMasterNotation->path());
    if (masterNotationDirPath != "") {
        suggestedPath = masterNotationDirPath;
    }

    suggestedPath += "/" + currentMasterNotation->metaInfo().title;

    if (unitType == WriterUnitType::MULTI_PART && !isExportingOnlyOneScore) {
        bool containsMaster = std::find_if(notations.cbegin(), notations.cend(), [this](INotationPtr notation) {
            return isMainNotation(notation);
        }) != notations.cend();

        if (containsMaster) {
            suggestedPath += "-" + qtrc("userscores", "Score_and_Parts", "Used in export filename suggestion");
        } else {
            suggestedPath += "-" + qtrc("userscores", "Parts", "Used in export filename suggestion");
        }
    } else if (isExportingOnlyOneScore) {
        if (!isMainNotation(notations.front())) {
            suggestedPath += "-" + io::escapeFileName(notations.front()->metaInfo().title);
        }

        if (isCreatingOnlyOneFile && unitType == WriterUnitType::PER_PAGE) {
            // So there is only one page
            suggestedPath += "-1";
        }
    }

    suggestedPath += "." + exportType.suffixes.front();

    io::path chosenPath = interactive()->selectSavingFile(qtrc("userscores", "Export"), suggestedPath,
                                                          exportType.filter(), isCreatingOnlyOneFile);
    if (chosenPath.empty()) {
        return false;
    }

    auto writer = writers()->writer(io::syffix(chosenPath));
    if (!writer) {
        return false;
    }

    IF_ASSERT_FAILED(writer->supportsUnitType(unitType)) {
        return false;
    }

    m_fileConflictPolicy = isCreatingOnlyOneFile ? FileConflictPolicy::ReplaceAll : FileConflictPolicy::Undefined;

    switch (unitType) {
    case WriterUnitType::PER_PAGE: {
        for (INotationPtr notation : notations) {
            for (int page = 0; page < notation->elements()->msScore()->pages().size(); page++) {
                INotationWriter::Options options {
                    { INotationWriter::OptionKey::UNIT_TYPE, Val(static_cast<int>(unitType)) },
                    { INotationWriter::OptionKey::PAGE_NUMBER, Val(page) },
                    { INotationWriter::OptionKey::TRANSPARENT_BACKGROUND, Val(
                          imagesExportConfiguration()->exportPngWithTransparentBackground()) }
                };

                io::path definitivePath = isCreatingOnlyOneFile
                                          ? chosenPath
                                          : configuration()->completeExportPath(chosenPath, notation, isMainNotation(notation), page);

                doExport(writer, { notation }, definitivePath, options);
            }
        }
    } break;
    case WriterUnitType::PER_PART: {
        for (INotationPtr notation : notations) {
            INotationWriter::Options options {
                { INotationWriter::OptionKey::UNIT_TYPE, Val(static_cast<int>(unitType)) },
                { INotationWriter::OptionKey::TRANSPARENT_BACKGROUND,
                  Val(imagesExportConfiguration()->exportPngWithTransparentBackground()) }
            };

            io::path definitivePath = isCreatingOnlyOneFile
                                      ? chosenPath
                                      : configuration()->completeExportPath(chosenPath, notation, isMainNotation(notation));

            doExport(writer, { notation }, definitivePath, options);
        }
    } break;
    case WriterUnitType::MULTI_PART: {
        INotationWriter::Options options {
            { INotationWriter::OptionKey::UNIT_TYPE, Val(static_cast<int>(unitType)) },
            { INotationWriter::OptionKey::TRANSPARENT_BACKGROUND, Val(imagesExportConfiguration()->exportPngWithTransparentBackground()) }
        };

        doExport(writer, notations, chosenPath, options);
    } break;
    }

    return true;
}

bool ExportScoreScenario::isCreatingOnlyOneFile(const INotationPtrList& notations, WriterUnitType unitType) const
{
    switch (unitType) {
    case WriterUnitType::PER_PAGE:
        return notations.size() == 1 && notations.front()->elements()->pages().size() == 1;
    case WriterUnitType::PER_PART:
        return notations.size() == 1;
    case WriterUnitType::MULTI_PART:
        return true;
    }

    return false;
}

bool ExportScoreScenario::isMainNotation(INotationPtr notation) const
{
    return context()->currentMasterNotation()->notation() == notation;
}

bool ExportScoreScenario::shouldReplaceFile(const QString& filename) const
{
    switch (m_fileConflictPolicy) {
    case FileConflictPolicy::ReplaceAll:
        return true;
    case FileConflictPolicy::SkipAll:
        return false;
    case FileConflictPolicy::Undefined: {
        static const int Replace = static_cast<int>(IInteractive::Button::CustomButton) + 1;
        static const int ReplaceAll = static_cast<int>(IInteractive::Button::CustomButton) + 2;
        static const int Skip = static_cast<int>(IInteractive::Button::CustomButton) + 3;
        static const int SkipAll = static_cast<int>(IInteractive::Button::CustomButton) + 4;

        int btn = interactive()->question(
            trc("userscores", "File already exists"),
            qtrc("userscores", "A file already exists with the filename %1. Do you want to replace it?")
            .arg(filename).toStdString(), {
                IInteractive::ButtonData(Replace, trc("userscores", "Replace")),
                IInteractive::ButtonData(ReplaceAll, trc("userscores", "Replace All")),
                IInteractive::ButtonData(Skip, trc("userscores", "Skip")),
                IInteractive::ButtonData(SkipAll, trc("userscores", "Skip All"))
            });

        switch (btn) {
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
}

bool ExportScoreScenario::askForRetry(const QString& filename) const
{
    int btn = interactive()->question(
        trc("userscores", "Error"),
        qtrc("userscores", "An error occured while writing the file %1. Do you want to retry?")
        .arg(filename).toStdString(), {
        interactive()->buttonData(IInteractive::Button::Retry),
        interactive()->buttonData(IInteractive::Button::Abort)
    });

    return btn == static_cast<int>(IInteractive::Button::Retry);
}

bool ExportScoreScenario::doExport(INotationWriterPtr writer, const INotationPtrList& notations, const io::path& path,
                                   const INotationWriter::Options& options) const
{
    QString filename = io::filename(path).toQString();
    if (fileSystem()->exists(path) && !shouldReplaceFile(filename)) {
        return false;
    }

    while (true) {
        QFile outputFile(path.toQString());
        if (!outputFile.open(QFile::WriteOnly)) {
            if (askForRetry(filename)) {
                continue;
            } else {
                return false;
            }
        }

        if (!writer->write(notations, outputFile, options)) {
            outputFile.close();
            if (askForRetry(filename)) {
                continue;
            } else {
                return false;
            }
        }

        outputFile.close();
        break;
    }

    return true;
}
