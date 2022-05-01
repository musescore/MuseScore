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
#ifndef MU_PROJECT_EXPORTPROJECTSCENARIO_H
#define MU_PROJECT_EXPORTPROJECTSCENARIO_H

#include "modularity/ioc.h"

#include "iexportprojectscenario.h"
#include "iprojectconfiguration.h"
#include "iinteractive.h"
#include "inotationwritersregister.h"
#include "importexport/imagesexport/iimagesexportconfiguration.h"
#include "context/iglobalcontext.h"
#include "system/ifilesystem.h"

namespace mu::project {
class ExportProjectScenario : public IExportProjectScenario
{
    INJECT(project, IProjectConfiguration, configuration)
    INJECT(project, framework::IInteractive, interactive)
    INJECT(project, INotationWritersRegister, writers)
    INJECT(project, iex::imagesexport::IImagesExportConfiguration, imagesExportConfiguration)
    INJECT(project, context::IGlobalContext, context)
    INJECT(project, system::IFileSystem, fileSystem)

public:
    std::vector<project::INotationWriter::UnitType> supportedUnitTypes(const ExportType& exportType) const override;

    bool exportScores(const notation::INotationPtrList& notations, const ExportType& exportType,
                      project::INotationWriter::UnitType unitType, bool openDestinationFolderOnExport = false) const override;

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

    void openFolder(const io::path& path) const;

    mutable FileConflictPolicy m_fileConflictPolicy;
};
}

#endif // MU_PROJECT_EXPORTPROJECTSCENARIO_H
