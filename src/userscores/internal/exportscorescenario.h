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

#ifndef MU_USERSCORES_EXPORTSCORESERVICE_H
#define MU_USERSCORES_EXPORTSCORESERVICE_H

#include "modularity/ioc.h"

#include "iexportscorescenario.h"
#include "iuserscoresconfiguration.h"
#include "iinteractive.h"
#include "notation/inotationwritersregister.h"
#include "importexport/imagesexport/iimagesexportconfiguration.h"
#include "context/iglobalcontext.h"
#include "system/ifilesystem.h"

namespace mu::userscores {
class ExportScoreScenario : public IExportScoreScenario
{
    INJECT(userscores, IUserScoresConfiguration, configuration);
    INJECT(userscores, framework::IInteractive, interactive);
    INJECT(userscores, notation::INotationWritersRegister, writers)
    INJECT(userscores, iex::imagesexport::IImagesExportConfiguration, imagesExportConfiguration)
    INJECT(userscores, context::IGlobalContext, context)
    INJECT(userscores, system::IFileSystem, fileSystem)

public:
    std::vector<ExportUnitType> supportedUnitTypes(const std::string& suffix) const override;

    bool exportScores(notation::INotationPtrList& notations, const std::string& suffix, ExportUnitType unitType) override;

private:
    enum class FileConflictPolicy {
        Undefined,
        SkipAll,
        ReplaceAll
    };

    bool isCreatingOnlyOneFile(notation::INotationPtrList& notations, ExportUnitType unitType) const;

    bool isMainNotation(notation::INotationPtr notation) const;

    QString fileFilter(const std::string& suffix);

    bool shouldReplaceFile(QString filename);
    bool askForRetry(QString filename) const;

    bool exportSingleScore(notation::INotationWriterPtr writer, io::path exportPath, bool pathIsDefinitive, notation::INotationPtr score,
                           int page = 0);
    bool exportScoreList(notation::INotationWriterPtr writer, io::path exportPath, notation::INotationPtrList notations);
    bool shouldExportIndividualPage(std::string suffix) const;

    FileConflictPolicy m_fileConflictPolicy;
};
}

#endif // MU_USERSCORES_EXPORTSCORESERVICE_H
