/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <gmock/gmock.h>

#include "project/iexportprojectscenario.h"

namespace mu::project {
class ExportProjectScenarioMock : public IExportProjectScenario
{
public:
    MOCK_METHOD(std::vector<INotationWriter::UnitType>, supportedUnitTypes, (const ExportType& exportType), (const, override));

    MOCK_METHOD(muse::RetVal<muse::io::path_t>, askExportPath,
                (const notation::INotationPtrList& notations, const ExportType& exportType, INotationWriter::UnitType unitType,
                 muse::io::path_t defaultPath), (const, override));

    MOCK_METHOD(bool, exportScores,
                (notation::INotationPtrList notations, const muse::io::path_t destinationPath, INotationWriter::UnitType unitType,
                 bool openDestinationFolderOnExport), (const, override));

    MOCK_METHOD(const ExportInfo&, exportInfo, (), (const, override));
    MOCK_METHOD(void, setExportInfo, (const ExportInfo& exportInfo), (override));
};
}
