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
#ifndef MU_SYSTEM_IFILESYSTEM_H
#define MU_SYSTEM_IFILESYSTEM_H

#include "modularity/imoduleexport.h"
#include "retval.h"
#include "io/path.h"

namespace mu::system {
class IFileSystem : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IFileSystem)

public:
    virtual ~IFileSystem() = default;

    virtual Ret exists(const io::path& path) const = 0;
    virtual Ret remove(const io::path& path) const = 0;
    virtual Ret copy(const io::path& src, const io::path& dst, bool replace = false) const = 0;
    virtual Ret move(const io::path& src, const io::path& dst, bool replace = false) const = 0;

    virtual Ret makePath(const io::path& path) const = 0;

    virtual RetVal<uint64_t> fileSize(const io::path& path) const = 0;

    enum class ScanMode {
        OnlyCurrentDir,
        IncludeSubdirs
    };

    virtual RetVal<io::paths> scanFiles(const io::path& rootDir, const QStringList& filters,
                                        ScanMode mode = ScanMode::IncludeSubdirs) const = 0;

    virtual RetVal<QByteArray> readFile(const io::path& filePath) const = 0;
    virtual Ret writeToFile(const io::path& filePath, const QByteArray& data) const = 0;

    enum class Attribute {
        Hidden
    };

    virtual void setAttribute(const io::path& path, Attribute attribute) const = 0;
};
}

#endif // MU_SYSTEM_IFILESYSTEM_H
