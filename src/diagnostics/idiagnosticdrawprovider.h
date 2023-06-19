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
#ifndef MU_DIAGNOSTICS_IDIAGNOSTICDRAWPROVIDER_H
#define MU_DIAGNOSTICS_IDIAGNOSTICDRAWPROVIDER_H

#include "modularity/imoduleinterface.h"
#include "global/types/ret.h"
#include "global/io/path.h"
#include "diagnosticstypes.h"

namespace mu::diagnostics {
class IDiagnosticDrawProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IDiagnosticDrawProvider)
public:
    virtual ~IDiagnosticDrawProvider() = default;

    virtual Ret generateDrawData(const io::path_t& scoresDir, const io::path_t& outDir, const GenOpt& opt = GenOpt()) = 0;
    virtual Ret compareDrawData(const io::path_t& ref, const io::path_t& test, const io::path_t& outDiff, const ComOpt& opt = ComOpt()) = 0;
    virtual Ret drawDataToPng(const io::path_t& dataFile, const io::path_t& outFile) = 0;
    virtual Ret drawDiffToPng(const io::path_t& diffFile, const io::path_t& refFile, const io::path_t& outFile) = 0;
};
}

#endif // MU_DIAGNOSTICS_IDIAGNOSTICDRAWPROVIDER_H
