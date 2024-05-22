/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#ifndef MU_ENGRAVING_DIAGNOSTICDRAWPROVIDER_H
#define MU_ENGRAVING_DIAGNOSTICDRAWPROVIDER_H

#include "idiagnosticdrawprovider.h"
#include "modularity/ioc.h"

namespace mu::engraving {
class DiagnosticDrawProvider : public IDiagnosticDrawProvider, public muse::Injectable
{
public:
    DiagnosticDrawProvider(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}

    muse::Ret generateDrawData(const muse::io::path_t& dirOrFile, const muse::io::path_t& outDirOrFile,
                               const GenOpt& opt = GenOpt()) override;
    muse::Ret compareDrawData(const muse::io::path_t& ref, const muse::io::path_t& test, const muse::io::path_t& outDiff,
                              const ComOpt& opt = ComOpt()) override;
    muse::Ret drawDataToPng(const muse::io::path_t& dataFile, const muse::io::path_t& outFile) override;
    muse::Ret drawDiffToPng(const muse::io::path_t& diffFile, const muse::io::path_t& refFile, const muse::io::path_t& outFile) override;
};
}

#endif // MU_ENGRAVING_DIAGNOSTICDRAWPROVIDER_H
