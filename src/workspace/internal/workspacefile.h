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
#ifndef MU_WORKSPACE_WORKSPACEFILE_H
#define MU_WORKSPACE_WORKSPACEFILE_H

#include "system/ifilesystem.h"
#include "modularity/ioc.h"

class MQZipReader;
class MQZipWriter;
class QByteArray;

namespace mu::workspace {
class WorkspaceFile
{
    INJECT(workspace, system::IFileSystem, fileSystem)

public:
    WorkspaceFile(const io::path& filePath);

    QByteArray readRootFile();
    bool writeRootFile(const std::string& name, const QByteArray& data);

private:
    struct MetaInfo
    {
        void write(MQZipWriter& zip);
        bool read(const MQZipReader& zip);

        void setRootFile(const std::string& name);
        std::string rootFile() const;

    private:
        void readContainer(const QByteArray& data);
        void writeContainer(QByteArray* data) const;

        std::string m_rootFile;
    };

    io::path m_filePath;
};
}

#endif // MU_WORKSPACE_WORKSPACEFILE_H
