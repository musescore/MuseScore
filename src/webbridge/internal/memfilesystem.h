/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#pragma once

#include <map>

#include "global/io/ifilesystem.h"

namespace mu::webbridge {
class MemFileSystem : public muse::io::IFileSystem
{
public:
    MemFileSystem() = default;

    muse::Ret exists(const muse::io::path_t& path) const override;
    muse::Ret remove(const muse::io::path_t& path, bool onlyIfEmpty = false) override;  // remove file or dir
    muse::Ret clear(const muse::io::path_t& path) override;  // clear dir
    muse::Ret copy(const muse::io::path_t& src, const muse::io::path_t& dst, bool replace = false) override;
    muse::Ret move(const muse::io::path_t& src, const muse::io::path_t& dst, bool replace = false) override;

    muse::Ret makePath(const muse::io::path_t& path) const override;
    muse::Ret makeLink(const muse::io::path_t& targetPath, const muse::io::path_t& linkPath) const override;

    muse::io::EntryType entryType(const muse::io::path_t& path) const override;

    muse::RetVal<uint64_t> fileSize(const muse::io::path_t& path) const override;

    muse::RetVal<muse::io::paths_t> scanFiles(const muse::io::path_t& rootDir, const std::vector<std::string>& filters,
                                              muse::io::ScanMode mode = muse::io::ScanMode::FilesInCurrentDirAndSubdirs) const override;

    void setAttribute(const muse::io::path_t& path, Attribute attribute) const override;
    bool setPermissionsAllowedForAll(const muse::io::path_t& path) const override;

    muse::RetVal<muse::ByteArray> readFile(const muse::io::path_t& filePath) const override;
    muse::Ret readFile(const muse::io::path_t& filePath, muse::ByteArray& data) const override;
    muse::Ret writeFile(const muse::io::path_t& filePath, const muse::ByteArray& data) override;

    //! NOTE File info
    muse::io::path_t canonicalFilePath(const muse::io::path_t& filePath) const override;
    muse::io::path_t absolutePath(const muse::io::path_t& filePath) const override;
    muse::io::path_t absoluteFilePath(const muse::io::path_t& filePath) const override;
    muse::DateTime birthTime(const muse::io::path_t& filePath) const override;
    muse::DateTime lastModified(const muse::io::path_t& filePath) const override;
    muse::Ret isWritable(const muse::io::path_t& filePath) const override;

private:

    std::map<muse::io::path_t, muse::ByteArray> m_files;
};
}
