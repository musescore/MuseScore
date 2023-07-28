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
#ifndef MU_DIAGNOSTICS_DIAGNOSTICFILESWRITER_H
#define MU_DIAGNOSTICS_DIAGNOSTICFILESWRITER_H

#include "modularity/ioc.h"
#include "io/ifilesystem.h"
#include "global/iglobalconfiguration.h"

#include "types/ret.h"
#include "io/path.h"

namespace mu {
class ZipWriter;
}

namespace mu::diagnostics {
class DiagnosticFilesWriter
{
    INJECT_STATIC(io::IFileSystem, fileSystem)
    INJECT_STATIC(framework::IGlobalConfiguration, globalConfiguration)

public:
    static Ret writeDiagnosticFiles(const io::path_t& destinationZipPath);

private:
    static RetVal<io::paths_t> scanDir(const std::string& dirName);
    static mu::Ret addFileToZip(const io::path_t& filePath, ZipWriter& zip, const std::string& destinationDirName = std::string());
};
}

#endif // MU_DIAGNOSTICS_DIAGNOSTICFILESWRITER_H
