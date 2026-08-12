/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include "modularity/imoduleinterface.h"
#include "notation/inotation_fwd.h"
#include "inotationwriter.h"
#include "internal/exporttype.h"

namespace mu::project {
struct ExportInfo {
    QString id;
    muse::io::path_t exportDirPath;
    INotationWriter::UnitType unitType;
    std::vector<notation::INotationWeakPtr> notations;
};

class IExportProjectScenario : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IExportProjectScenario)

public:
    virtual std::vector<INotationWriter::UnitType> supportedUnitTypes(const ExportType& exportType) const = 0;

    virtual muse::RetVal<muse::io::path_t> askExportPath(const notation::INotationPtrList& notations, const ExportType& exportType,
                                                         INotationWriter::UnitType unitType = INotationWriter::UnitType::PER_PART,
                                                         muse::io::path_t defaultPath = "") const = 0;

    virtual bool exportScores(notation::INotationPtrList notations, const muse::io::path_t destinationPath,
                              INotationWriter::UnitType unitType = INotationWriter::UnitType::PER_PART,
                              bool openDestinationFolderOnExport = false) const = 0;

    virtual const ExportInfo& exportInfo() const = 0;
    virtual void setExportInfo(const ExportInfo& exportInfo) = 0;
};
}
