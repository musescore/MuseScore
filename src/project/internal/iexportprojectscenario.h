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
#ifndef MU_PROJECT_IEXPORTPROJECTSCENARIO_H
#define MU_PROJECT_IEXPORTPROJECTSCENARIO_H

#include "modularity/imoduleinterface.h"
#include "notation/inotation.h"
#include "inotationwriter.h"
#include "exporttype.h"

namespace mu::project {
class IExportProjectScenario : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IExportProjectScenario)

public:
    virtual std::vector<INotationWriter::UnitType> supportedUnitTypes(const ExportType& exportType) const = 0;

    virtual RetVal<io::path_t> askExportPath(const notation::INotationPtrList& notations, const ExportType& exportType,
                                             INotationWriter::UnitType unitType = INotationWriter::UnitType::PER_PART) const = 0;

    virtual bool exportScores(const notation::INotationPtrList& notations, const io::path_t& destinationPath,
                              INotationWriter::UnitType unitType = INotationWriter::UnitType::PER_PART,
                              bool openDestinationFolderOnExport = false) const = 0;
};
}

#endif // MU_PROJECT_IEXPORTPROJECTSCENARIO_H
