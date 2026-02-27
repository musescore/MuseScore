/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_PROJECT_IEXPORTPROJECTSCENARIO_H
#define MU_PROJECT_IEXPORTPROJECTSCENARIO_H

#include "modularity/imoduleinterface.h"
#include "notation/inotation.h"
#include "iprojectwriter.h"
#include "internal/exporttype.h"
#include "types/projecttypes.h"

namespace mu::project {
class IExportProjectScenario : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IExportProjectScenario)

public:
    virtual std::vector<WriteUnitType> supportedUnitTypes(const ExportType& exportType) const = 0;

    virtual muse::RetVal<muse::io::path_t> askExportPath(const notation::INotationPtrList& notations, const ExportType& exportType,
                                                         WriteUnitType unitType = WriteUnitType::PER_PART,
                                                         muse::io::path_t defaultPath = "") const = 0;

    virtual bool exportScores(notation::INotationPtrList notations, const muse::io::path_t destinationPath,
                              WriteUnitType unitType = WriteUnitType::PER_PART,
                              bool openDestinationFolderOnExport = false) const = 0;

    virtual const ExportInfo& exportInfo() const = 0;
    virtual void setExportInfo(const ExportInfo& exportInfo) = 0;
};
}

#endif // MU_PROJECT_IEXPORTPROJECTSCENARIO_H
