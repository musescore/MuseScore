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
#ifndef MUSE_WORKSPACE_WORKSPACEFILE_H
#define MUSE_WORKSPACE_WORKSPACEFILE_H

#include <string>
#include <map>
#include <QByteArray>

#include "io/ifilesystem.h"
#include "modularity/ioc.h"
#include "types/val.h"

namespace muse {
class ZipReader;
class ZipWriter;
}

namespace muse::workspace {
class WorkspaceFile
{
    GlobalInject<io::IFileSystem> fileSystem;

public:
    WorkspaceFile(const io::path_t& filePath);
    WorkspaceFile(const io::path_t& filePath, const WorkspaceFile* other);

    io::path_t filePath() const;
    void redirect(const io::path_t& filePath);

    Ret load();
    Ret save();
    bool isLoaded() const;
    bool isNeedSave() const;

    Val meta(const std::string& key) const;
    void setMeta(const std::string& key, const Val& val);

    QByteArray data(const std::string& name) const;
    void setData(const std::string& name, const QByteArray& data);

private:

    struct Container
    {
        static void write(ZipWriter& zip, const std::vector<std::string>& paths);
    };

    struct Meta
    {
        static void write(ZipWriter& zip, const std::map<std::string, Val>& meta);
        static void read(ZipReader& zip, std::map<std::string, Val>& meta);
    };

    void markDirty();

    io::path_t m_filePath;
    std::map<std::string, Val> m_meta;
    std::map<std::string, QByteArray> m_data;

    std::atomic<bool> m_needSave = false;
};
}

#endif // MUSE_WORKSPACE_WORKSPACEFILE_H
