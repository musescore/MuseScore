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
#ifndef MU_IO_FILEINFO_H
#define MU_IO_FILEINFO_H

#include "modularity/ioc.h"
#include "ifilesystem.h"

namespace mu::io {
class FileInfo
{
    INJECT_STATIC(io, IFileSystem, fileSystem)
public:
    FileInfo() = default;
    FileInfo(const path_t& filePath);

    QString path() const;
    QString filePath() const;
    QString canonicalFilePath() const;
    QString absolutePath() const;

    QString fileName() const;
    QString baseName() const;
    QString completeBaseName() const;
    QString suffix() const;
    static QString suffix(const path_t& filePath);

    bool isRelative() const;
    bool isAbsolute() const;

    bool exists() const;
    static bool exists(const path_t& filePath);

    DateTime birthTime() const;
    DateTime lastModified() const;

private:
    static QString doSuffix(const QString& filePath);

    QString m_filePath;
};
}

#endif // MU_IO_FILEINFO_H
