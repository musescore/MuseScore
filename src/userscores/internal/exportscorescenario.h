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
    std::vector<notation::WriterUnitType> supportedUnitTypes(const ExportType& exportType) const override;

    bool exportScores(const notation::INotationPtrList& notations, const ExportType& exportType,
                      notation::WriterUnitType unitType) const override;

private:
    enum class FileConflictPolicy {
        Undefined,
        SkipAll,
        ReplaceAll
    };

    bool isCreatingOnlyOneFile(const notation::INotationPtrList& notations, notation::WriterUnitType unitType) const;

    bool isMainNotation(notation::INotationPtr notation) const;

    bool shouldReplaceFile(const QString& filename) const;
    bool askForRetry(const QString& filename) const;

    bool doExport(notation::INotationWriterPtr writer, const notation::INotationPtrList& notations, const io::path& path,
                  const notation::INotationWriter::Options& options) const;

    mutable FileConflictPolicy m_fileConflictPolicy;
};
}

#endif // MU_USERSCORES_EXPORTSCORESERVICE_H
