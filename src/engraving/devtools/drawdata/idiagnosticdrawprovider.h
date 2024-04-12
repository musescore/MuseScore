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
#ifndef MU_ENGRAVING_IDIAGNOSTICDRAWPROVIDER_H
#define MU_ENGRAVING_IDIAGNOSTICDRAWPROVIDER_H

#include "modularity/imoduleinterface.h"
#include "global/types/ret.h"
#include "global/io/path.h"

#include "drawdatatypes.h"

namespace mu::engraving {
class IDiagnosticDrawProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IDiagnosticDrawProvider)
public:
    virtual ~IDiagnosticDrawProvider() = default;

    virtual muse::Ret generateDrawData(const muse::io::path_t& scoresDir, const muse::io::path_t& outDir, const GenOpt& opt = GenOpt()) = 0;
    virtual muse::Ret compareDrawData(const muse::io::path_t& ref, const muse::io::path_t& test, const muse::io::path_t& outDiff,
                                      const ComOpt& opt = ComOpt()) = 0;
    virtual muse::Ret drawDataToPng(const muse::io::path_t& dataFile, const muse::io::path_t& outFile) = 0;
    virtual muse::Ret drawDiffToPng(const muse::io::path_t& diffFile, const muse::io::path_t& refFile, const muse::io::path_t& outFile) = 0;
};
}

#endif // MU_ENGRAVING_IDIAGNOSTICDRAWPROVIDER_H
