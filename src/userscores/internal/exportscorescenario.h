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
#ifndef MU_USERSCORES_EXPORTSCORESERVICE_H
#define MU_USERSCORES_EXPORTSCORESERVICE_H

#include "modularity/ioc.h"

#include "iexportscorescenario.h"
#include "iuserscoresconfiguration.h"
#include "iinteractive.h"
#include "project/inotationwritersregister.h"
#include "importexport/imagesexport/iimagesexportconfiguration.h"
#include "context/iglobalcontext.h"
#include "system/ifilesystem.h"

namespace mu::userscores {
class ExportScoreScenario : public IExportScoreScenario
{
    INJECT(userscores, IUserScoresConfiguration, configuration)
    INJECT(userscores, framework::IInteractive, interactive)
    INJECT(userscores, project::INotationWritersRegister, writers)
    INJECT(userscores, iex::imagesexport::IImagesExportConfiguration, imagesExportConfiguration)
    INJECT(userscores, context::IGlobalContext, context)
    INJECT(userscores, system::IFileSystem, fileSystem)

public:
    std::vector<project::INotationWriter::UnitType> supportedUnitTypes(const ExportType& exportType) const override;

    bool exportScores(const notation::INotationPtrList& notations, const ExportType& exportType,
                      project::INotationWriter::UnitType unitType) const override;

private:
    enum class FileConflictPolicy {
        Undefined,
        SkipAll,
        ReplaceAll
    };

    bool isCreatingOnlyOneFile(const notation::INotationPtrList& notations, project::INotationWriter::UnitType unitType) const;

    bool isMainNotation(notation::INotationPtr notation) const;

    io::path askExportPath(const notation::INotationPtrList& notations, const ExportType& exportType,
                           project::INotationWriter::UnitType unitType) const;
    io::path completeExportPath(const io::path& basePath, notation::INotationPtr notation, bool isMain, int pageIndex = -1) const;

    bool shouldReplaceFile(const QString& filename) const;
    bool askForRetry(const QString& filename) const;

    bool doExportLoop(const io::path& path, std::function<bool(io::Device&)> exportFunction) const;

    mutable FileConflictPolicy m_fileConflictPolicy;
};
}

#endif // MU_USERSCORES_EXPORTSCORESERVICE_H
