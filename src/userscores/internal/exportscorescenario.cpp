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

std::vector<ExportUnitType> ExportScoreScenario::supportedUnitTypes(const std::string& suffix) const
{
    auto writer = writers()->writer(suffix);
    if (!writer) {
        return {};
    }

    return writer->supportedUnitTypes();
}

bool ExportScoreScenario::exportScores(INotationPtrList& notations, const std::string& suffix, ExportUnitType unitType)
{
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

    if (unitType == ExportUnitType::MULTI_PART && !isExportingOnlyOneScore) {
        bool containsMaster = false;
        for (auto notation : notations) {
            if (isMainNotation(notation)) {
                containsMaster = true;
                break;
            }
        }

        if (containsMaster) {
            suggestedPath += "-" + qtrc("userscores", "Score_and_Parts", "Used in export filename suggestion");
        } else {
            suggestedPath += "-" + qtrc("userscores", "Parts", "Used in export filename suggestion");
        }
    } else if (isExportingOnlyOneScore) {
        if (!isMainNotation(notations.front())) {
            suggestedPath += "-" + io::escapeFileName(notations.front()->metaInfo().title);
        }

        if (isCreatingOnlyOneFile && unitType == ExportUnitType::PER_PAGE) {
            // So there is only one page
            suggestedPath += "-1";
        }
    }

    suggestedPath += "." + suffix;

    io::path exportPath = interactive()->selectSavingFile(qtrc("userscores", "Export"),
                                                          suggestedPath, fileFilter(suffix), isCreatingOnlyOneFile);
    if (exportPath.empty()) {
        return false;
    }

    auto writer = writers()->writer(io::syffix(exportPath));
    if (!writer) {
        return false;
    }

    m_fileConflictPolicy = isCreatingOnlyOneFile ? FileConflictPolicy::ReplaceAll : FileConflictPolicy::Undefined;

    switch (unitType) {
    case ExportUnitType::PER_PAGE:
        for (INotationPtr currentNotation : notations) {
            for (int page = 0; page < currentNotation->elements()->msScore()->pages().size(); page++) {
                exportSingleScore(writer, exportPath, isCreatingOnlyOneFile, currentNotation, page);
            }
        }
        break;
    case ExportUnitType::PER_PART:
        for (INotationPtr currentNotation : notations) {
            exportSingleScore(writer, exportPath, isCreatingOnlyOneFile, currentNotation);
        }
        break;
    case ExportUnitType::MULTI_PART:
        exportScoreList(writer, exportPath, notations);
        break;
    }

    return true;
}

bool ExportScoreScenario::isCreatingOnlyOneFile(INotationPtrList& notations, INotationWriter::UnitType unitType) const
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

QString ExportScoreScenario::fileFilter(const std::string& suffix)
{
    QString filter;

    if (suffix == "pdf") {
        filter = qtrc("userscores", "PDF Files") + " (*.pdf)";
    } else if (suffix == "png") {
        filter = qtrc("userscores", "PNG Images") + " (*.png)";
    } else if (suffix == "svg") {
        filter = qtrc("userscores", "SVG Images") + " (*.svg)";
    } else if (suffix == "mp3") {
        filter = qtrc("userscores", "MP3 Audio Files") + " (*.mp3)";
    } else if (suffix == "wav") {
        filter = qtrc("userscores", "WAV Audio Files") + " (*.wav)";
    } else if (suffix == "ogg") {
        filter = qtrc("userscores", "OGG Audio Files") + " (*.ogg)";
    } else if (suffix == "flac") {
        filter = qtrc("userscores", "FLAC Audio Files") + " (*.flac)";
    } else if (suffix == "mid" || suffix == "midi" || suffix == "kar") {
        filter = qtrc("userscores", "MIDI Files") + " (*.mid *.midi *.kar)";
    } else if (suffix == "mxl") {
        filter = qtrc("userscores", "Compressed MusicXML Files") + " (*.mxl)";
    } else if (suffix == "musicxml" || suffix == "xml") {
        filter = qtrc("userscores", "Uncompressed MusicXML Files") + " (*.musicxml *.xml)";
    }

    return filter;
}

bool ExportScoreScenario::askForRetry(QString filename) const
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

bool ExportScoreScenario::shouldReplaceFile(QString filename)
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

bool ExportScoreScenario::exportSingleScore(INotationWriterPtr writer, io::path exportPath, bool pathIsDefinitive, INotationPtr notation,
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

bool ExportScoreScenario::exportScoreList(INotationWriterPtr writer, io::path exportPath, INotationPtrList notations)
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

bool ExportScoreScenario::shouldExportIndividualPage(std::string suffix) const
{
    static const std::vector<INotationWriter::UnitType> perPageOnly = { INotationWriter::UnitType::PER_PAGE };
    return supportedUnitTypes(suffix) == perPageOnly;
}

bool ExportScoreScenario::isMainNotation(INotationPtr notation) const
{
    return context()->currentMasterNotation()->notation() == notation;
}
