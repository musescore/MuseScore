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
#ifndef MU_IO_DIR_H
#define MU_IO_DIR_H

#include "types/retval.h"
#include "path.h"
#include "ioenums.h"

#include "modularity/ioc.h"
#include "ifilesystem.h"

namespace mu::io {
class IFileSystem;
class Dir
{
    INJECT_STATIC(IFileSystem, fileSystem)

public:
    Dir() = default;
    Dir(const path_t& path);

    path_t path() const;
    path_t absolutePath() const;

    bool exists() const;
    Ret removeRecursively();

    Ret mkpath();
    static Ret mkpath(const path_t& path);

    static RetVal<io::paths_t> scanFiles(const io::path_t& rootDir, const std::vector<std::string>& filters,
                                         ScanMode mode = ScanMode::FilesInCurrentDirAndSubdirs);

    static path_t fromNativeSeparators(const path_t& pathName);

private:
    path_t m_path;
};
}

#endif // MU_IO_DIR_H
