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
#ifndef MUSE_DIAGNOSTICS_DIAGNOSTICFILESWRITER_H
#define MUSE_DIAGNOSTICS_DIAGNOSTICFILESWRITER_H

#include "modularity/ioc.h"
#include "io/ifilesystem.h"
#include "global/iglobalconfiguration.h"

#include "types/ret.h"
#include "io/path.h"

namespace muse {
class ZipWriter;
}

namespace muse::diagnostics {
class DiagnosticFilesWriter
{
    INJECT_STATIC(muse::io::IFileSystem, fileSystem)
    INJECT_STATIC(muse::IGlobalConfiguration, globalConfiguration)

public:
    static muse::Ret writeDiagnosticFiles(const muse::io::path_t& destinationZipPath);

private:
    static muse::RetVal<muse::io::paths_t> scanDir(const std::string& dirName);
    static muse::Ret addFileToZip(const muse::io::path_t& filePath, muse::ZipWriter& zip,
                                  const std::string& destinationDirName = std::string());
};
}

#endif // MUSE_DIAGNOSTICS_DIAGNOSTICFILESWRITER_H
