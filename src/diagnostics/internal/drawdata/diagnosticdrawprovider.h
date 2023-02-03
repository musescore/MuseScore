/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#ifndef MU_DIAGNOSTICS_DIAGNOSTICDRAWPROVIDER_H
#define MU_DIAGNOSTICS_DIAGNOSTICDRAWPROVIDER_H

#include "../../idiagnosticdrawprovider.h"

namespace mu::diagnostics {
class DiagnosticDrawProvider : public IDiagnosticDrawProvider
{
public:
    DiagnosticDrawProvider() = default;

    Ret generateDrawData(const io::path_t& dirOrFile, const io::path_t& outDirOrFile, const GenOpt& opt = GenOpt()) override;
    Ret compareDrawData(const io::path_t& ref, const io::path_t& test, const io::path_t& outDiff, const ComOpt& opt = ComOpt()) override;
    Ret drawDataToPng(const io::path_t& dataFile, const io::path_t& outFile) override;
    Ret drawDiffToPng(const io::path_t& diffFile, const io::path_t& refFile, const io::path_t& outFile) override;
};
}

#endif // MU_DIAGNOSTICS_DIAGNOSTICDRAWPROVIDER_H
