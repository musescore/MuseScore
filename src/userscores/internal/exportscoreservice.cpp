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

#include "log.h"
#include "translation.h"

using namespace mu;
using namespace mu::userscores;
using namespace mu::notation;
using namespace mu::framework;

void ExportScoreService::exportScores(QList<INotationPtr> notations, io::path exportPath)
{
    std::string suffix = io::syffix(exportPath);

    auto writer = writers()->writer(suffix);
    if (!writer) {
        return;
    }

    for (INotationPtr currentNotation : notations) {
        if (shouldExportIndividualPage(suffix)) {
            for (int page = 0; page < currentNotation->elements()->msScore()->pages().size(); page++) {
                exportSingleScore(writer, exportPath, currentNotation, page);
            }
        } else {
            exportSingleScore(writer, exportPath, currentNotation);
        }
    }
}

ExportScoreService::FileConflictPolicy ExportScoreService::getConflictPolicy(std::string filename)
{
    switch (m_currentConflictPolicy) {
    case Replace:
    case Skip:
        askConflictPolicy(filename);
    }

    return getEffectivePolicy(m_currentConflictPolicy);
}

bool ExportScoreService::askForRetry(std::string filename) const
{
    int btn = interactive()->question("Error", "An error occured while exporting the file " + filename, {
        interactive()->buttonData(IInteractive::Button::Retry),
        interactive()->buttonData(IInteractive::Button::Abort)
    });

    return btn == static_cast<int>(IInteractive::Button::Retry);
}

ExportScoreService::FileConflictPolicy ExportScoreService::getEffectivePolicy(ExportScoreService::FileConflictPolicy policy) const
{
    switch (policy) {
    case ReplaceAll:
        return FileConflictPolicy::Replace;
    case SkipAll:
        return FileConflictPolicy::Skip;
    default:
        return policy;
    }
}

void ExportScoreService::askConflictPolicy(std::string filename)
{
    int replace = static_cast<int>(IInteractive::Button::CustomButton) + 1;
    int replaceAll = static_cast<int>(IInteractive::Button::CustomButton) + 2;
    int skip = static_cast<int>(IInteractive::Button::CustomButton) + 3;
    int skipAll = static_cast<int>(IInteractive::Button::CustomButton) + 4;

    int btn = interactive()->question("File already exists", "A file already exists with the filename " + filename, {
        IInteractive::ButtonData(replace, "Replace"),
        IInteractive::ButtonData(replaceAll, "Replace All"),
        IInteractive::ButtonData(skip, "Skip"),
        IInteractive::ButtonData(skipAll, "Skip All")
    });

    m_currentConflictPolicy = static_cast<FileConflictPolicy>(btn - replace);
}

bool ExportScoreService::exportSingleScore(INotationWriterPtr writer, io::path exportPath, INotationPtr notation, int page)
{
    io::path outPath
        = configuration()->completeExportPath(exportPath, notation, isMainNotation(notation), shouldExportIndividualPage(io::syffix(
                                                                                                                             exportPath)),
                                              page);
    QString outPathString = outPath.toQString();
    std::string completeFileName = io::filename(outPath).toStdString();

    if (fileSystem()->exists(outPathString)) {
        if (getConflictPolicy(completeFileName)) {
            return false;
        }
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

bool ExportScoreService::shouldExportIndividualPage(io::path type) const
{
    return type == "png" || type == "svg";
}

bool ExportScoreService::isMainNotation(INotationPtr notation) const
{
    return context()->currentMasterNotation()->notation() == notation;
}
