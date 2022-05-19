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
#include "zipreader.h"

#include <QBuffer>

#include "internal/qzipreader_p.h"

using namespace mu;

struct ZipReader::Impl
{
    MQZipReader* zip = nullptr;
    io::ByteArray data;
    QByteArray ba;
    QBuffer buf;
};

ZipReader::ZipReader(QIODevice* device)
{
    m_impl = new Impl();
    m_impl->zip = new MQZipReader(device);
}

ZipReader::ZipReader(io::IODevice* device)
{
    m_impl = new Impl();
    m_impl->data = device->readAll();
    m_impl->ba = m_impl->data.toQByteArrayNoCopy();
    m_impl->buf.setBuffer(&m_impl->ba);
    m_impl->buf.open(QIODevice::ReadOnly);
    m_impl->zip = new MQZipReader(&m_impl->buf);
}

ZipReader::~ZipReader()
{
    close();
    delete m_impl->zip;
    delete m_impl;
}

void ZipReader::close()
{
    m_impl->zip->close();
}

ZipReader::Status ZipReader::status() const
{
    return static_cast<Status>(m_impl->zip->status());
}

std::vector<ZipReader::FileInfo> ZipReader::fileInfoList() const
{
    std::vector<FileInfo> ret;
    QVector<MQZipReader::FileInfo> qfis = m_impl->zip->fileInfoList();
    ret.reserve(qfis.size());
    for (const MQZipReader::FileInfo& qfi : qfis) {
        FileInfo fi;
        fi.filePath = qfi.filePath;
        fi.isDir = qfi.isDir;
        fi.isFile = qfi.isFile;
        fi.isSymLink = qfi.isSymLink;
        fi.size = qfi.size;

        ret.push_back(std::move(fi));
    }

    return ret;
}

QByteArray ZipReader::fileData(const QString& fileName) const
{
    return m_impl->zip->fileData(fileName);
}
