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

#include <QDateTime>

#include "modularity/imoduleexport.h"
#include "retval.h"
#include "path.h"
#include "types/bytearray.h"

namespace mu::io {
class IFileSystem : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IFileSystem)

public:
    virtual ~IFileSystem() = default;

    virtual Ret exists(const io::path_t& path) const = 0;
    virtual Ret remove(const io::path_t& path) const = 0;
    virtual Ret removeFolderIfEmpty(const io::path_t& path) const = 0;
    virtual Ret copy(const io::path_t& src, const io::path_t& dst, bool replace = false) const = 0;
    virtual Ret move(const io::path_t& src, const io::path_t& dst, bool replace = false) const = 0;

    virtual Ret makePath(const io::path_t& path) const = 0;

    virtual RetVal<uint64_t> fileSize(const io::path_t& path) const = 0;

    enum class ScanMode {
        FilesInCurrentDir,
        FilesAndFoldersInCurrentDir,
        FilesInCurrentDirAndSubdirs
    };

    virtual RetVal<io::paths_t> scanFiles(const io::path_t& rootDir, const QStringList& filters,
                                          ScanMode mode = ScanMode::FilesInCurrentDirAndSubdirs) const = 0;

    enum class Attribute {
        Hidden
    };

    virtual void setAttribute(const io::path_t& path, Attribute attribute) const = 0;
    virtual bool setPermissionsAllowedForAll(const io::path_t& path) const = 0;

    virtual RetVal<QByteArray> readFile(const io::path_t& filePath) const = 0;
    virtual Ret writeToFile(const io::path_t& filePath, const QByteArray& data) const = 0;

    virtual bool readFile(const io::path_t& filePath, ByteArray& data) const = 0;
    virtual bool writeFile(const io::path_t& filePath, const ByteArray& data) const = 0;

    //! NOTE File info
    virtual io::path_t canonicalFilePath(const io::path_t& filePath) const = 0;
    virtual io::path_t absolutePath(const io::path_t& filePath) const = 0;
    virtual QDateTime birthTime(const io::path_t& filePath) const = 0;
    virtual QDateTime lastModified(const io::path_t& filePath) const = 0;
    virtual bool isWritable(const io::path_t& filePath) const = 0;
};
}

#endif // MU_SYSTEM_IFILESYSTEM_H
