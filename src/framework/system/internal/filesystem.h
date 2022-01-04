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

#include "../ifilesystem.h"

namespace mu::system {
class FileSystem : public IFileSystem
{
public:
    Ret exists(const io::path& path) const override;
    Ret remove(const io::path& path) const override;
    Ret copy(const io::path& src, const io::path& dst, bool replace = false) const override;
    Ret move(const io::path& src, const io::path& dst, bool replace = false) const override;

    Ret makePath(const io::path& path) const override;

    RetVal<uint64_t> fileSize(const io::path& path) const override;

    RetVal<io::paths> scanFiles(const io::path& rootDir, const QStringList& filters,
                                ScanMode mode = ScanMode::IncludeSubdirs) const override;

    RetVal<QByteArray> readFile(const io::path& filePath) const override;
    Ret writeToFile(const io::path& filePath, const QByteArray& data) const override;

    void setAttribute(const io::path& path, Attribute attribute) const override;

private:
    Ret removeFile(const io::path& path) const;
    Ret removeDir(const io::path& path) const;
    Ret copyRecursively(const io::path& src, const io::path& dst) const;
};
}

#endif // MU_SYSTEM_FILESYSTEM_H
