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
#ifndef MU_SYSTEM_FILESYSTEM_H
#define MU_SYSTEM_FILESYSTEM_H

#include <QDateTime>

#include "../ifilesystem.h"

namespace mu::io {
class FileSystem : public IFileSystem
{
public:
    Ret exists(const io::path_t& path) const override;
    Ret remove(const io::path_t& path) const override;
    Ret removeFolderIfEmpty(const io::path_t& path) const override;
    Ret copy(const io::path_t& src, const io::path_t& dst, bool replace = false) const override;
    Ret move(const io::path_t& src, const io::path_t& dst, bool replace = false) const override;

    Ret makePath(const io::path_t& path) const override;

    RetVal<uint64_t> fileSize(const io::path_t& path) const override;

    RetVal<io::paths_t> scanFiles(const io::path_t& rootDir, const QStringList& filters,
                                  ScanMode mode = ScanMode::FilesInCurrentDirAndSubdirs) const override;

    RetVal<QByteArray> readFile(const io::path_t& filePath) const override;
    Ret writeToFile(const io::path_t& filePath, const QByteArray& data) const override;

    bool readFile(const io::path_t& filePath, ByteArray& data) const override;
    bool writeFile(const io::path_t& filePath, const ByteArray& data) const override;

    void setAttribute(const io::path_t& path, Attribute attribute) const override;

    io::path_t canonicalFilePath(const io::path_t& filePath) const override;
    io::path_t absolutePath(const io::path_t& filePath) const override;
    QDateTime birthTime(const io::path_t& filePath) const override;
    QDateTime lastModified(const io::path_t& filePath) const override;
    bool isWritable(const path_t& filePath) const override;

private:
    Ret removeFile(const io::path_t& path) const;
    Ret removeDir(const io::path_t& path, bool recursively = true) const;
    Ret copyRecursively(const io::path_t& src, const io::path_t& dst) const;
};
}

#endif // MU_SYSTEM_FILESYSTEM_H
