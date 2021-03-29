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

#include "exportscoreservice.h"

#include "translation.h"

using namespace mu::userscores;
using namespace mu::notation;
using namespace mu::framework;

std::vector<ExportUnitType> ExportScoreService::supportedUnitTypes(const std::string& suffix) const
{
    auto writer = writers()->writer(suffix);
    if (!writer) {
        return {};
    }

    return writer->supportedUnitTypes();
}

bool ExportScoreService::willCreateOnlyOneFile(INotationWriter::UnitType unitType, INotationPtrList& notations) const
{
    switch (unitType) {
    case ExportUnitType::PER_PAGE:
        return notations.size() == 1 && notations.front()->elements()->pages().size() == 1;
    case ExportUnitType::PER_PART:
        return notations.size() == 1;
    case ExportUnitType::MULTI_PART:
        return true;
    }
}

void ExportScoreService::exportScores(INotationPtrList& notations, io::path& exportPath, ExportUnitType unitType)
{
    std::string suffix = io::syffix(exportPath);

    auto writer = writers()->writer(suffix);
    if (!writer) {
        return;
    }

    bool oneFile = willCreateOnlyOneFile(unitType, notations);
    m_fileConflictPolicy = oneFile ? FileConflictPolicy::ReplaceAll : FileConflictPolicy::Undefined;

    switch (unitType) {
    case ExportUnitType::PER_PAGE:
        for (INotationPtr currentNotation : notations) {
            for (int page = 0; page < currentNotation->elements()->msScore()->pages().size(); page++) {
                exportSingleScore(writer, exportPath, oneFile, currentNotation, page);
            }
        }
        break;
    case ExportUnitType::PER_PART:
        for (INotationPtr currentNotation : notations) {
            exportSingleScore(writer, exportPath, oneFile, currentNotation);
        }
        break;
    case ExportUnitType::MULTI_PART:
        exportScoreList(writer, exportPath, notations);
        break;
    }
}

bool ExportScoreService::askForRetry(QString filename) const
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

bool ExportScoreService::shouldReplaceFile(QString filename)
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

bool ExportScoreService::exportSingleScore(INotationWriterPtr writer, io::path exportPath, bool pathIsDefinitive, INotationPtr notation,
                                           int page)
{
    io::path outPath;
    if (pathIsDefinitive) {
        outPath = exportPath;
    } else {
        outPath = configuration()->completeExportPath(exportPath, notation, isMainNotation(notation),
                                                      shouldExportIndividualPage(io::syffix(exportPath)), page);
    }

    QString completeFileName = io::filename(outPath).toQString();

    if (fileSystem()->exists(outPath) && !shouldReplaceFile(completeFileName)) {
        return false;
    }

    INotationWriter::Options options({
        { INotationWriter::OptionKey::PAGE_NUMBER, Val(page) },
        { INotationWriter::OptionKey::TRANSPARENT_BACKGROUND, Val(imagesExportConfiguration()->exportPngWithTransparentBackground()) }
    });

    while (true) {
        QFile outFile(outPath.toQString());
        if (!outFile.open(QFile::WriteOnly)) {
            if (askForRetry(completeFileName)) {
                continue;
            } else {
                return false;
            }
        }

        if (!writer->write(notation, outFile, options)) {
            outFile.close();
            if (askForRetry(completeFileName)) {
                continue;
            } else {
                return false;
            }
        }

        outFile.close();
        break;
    }

    return true;
}

bool ExportScoreService::exportScoreList(INotationWriterPtr writer, io::path exportPath, INotationPtrList notations)
{
    QString completeFileName = io::filename(exportPath).toQString();

    INotationWriter::Options options({
        { INotationWriter::OptionKey::TRANSPARENT_BACKGROUND, Val(imagesExportConfiguration()->exportPngWithTransparentBackground()) }
    });

    while (true) {
        QFile outFile(exportPath.toQString());
        if (!outFile.open(QFile::WriteOnly)) {
            if (askForRetry(completeFileName)) {
                continue;
            } else {
                return false;
            }
        }

        if (!writer->writeList(notations, outFile, options)) {
            outFile.close();
            if (askForRetry(completeFileName)) {
                continue;
            } else {
                return false;
            }
        }

        outFile.close();
        break;
    }

    return true;
}

bool ExportScoreService::shouldExportIndividualPage(std::string suffix) const
{
    static const std::vector<INotationWriter::UnitType> perPageOnly = { INotationWriter::UnitType::PER_PAGE };
    return supportedUnitTypes(suffix) == perPageOnly;
}

bool ExportScoreService::isMainNotation(INotationPtr notation) const
{
    return context()->currentMasterNotation()->notation() == notation;
}
