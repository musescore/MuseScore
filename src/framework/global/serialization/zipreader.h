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
#ifndef MU_GLOBAL_ZIPREADER_H
#define MU_GLOBAL_ZIPREADER_H

#include <vector>

#include <QIODevice>
#include "io/iodevice.h"

namespace mu {
class ZipReader
{
public:

    enum Status {
        NoError,
        FileReadError,
        FileOpenError,
        FilePermissionsError,
        FileError
    };

    struct FileInfo
    {
        QString filePath;
        bool isDir = false;
        bool isFile = false;
        bool isSymLink = false;
        qint64 size = 0;

        bool isValid() const { return isDir || isFile || isSymLink; }
    };

    explicit ZipReader(QIODevice* device);
    explicit ZipReader(io::IODevice* device);
    ~ZipReader();

    void close();
    Status status() const;

    std::vector<FileInfo> fileInfoList() const;
    QByteArray fileData(const QString& fileName) const;

private:
    struct Impl;
    Impl* m_impl = nullptr;
    io::IODevice* m_device = nullptr;
};
}

#endif // MU_GLOBAL_ZIPREADER_H
