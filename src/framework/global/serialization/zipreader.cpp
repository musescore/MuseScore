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
#include "io/file.h"

using namespace mu;
using namespace mu::io;

struct ZipReader::Impl
{
    MQZipReader* zip = nullptr;
    ByteArray data;
    QByteArray ba;
    QBuffer buf;
};

ZipReader::ZipReader(const io::path_t& filePath)
    : m_filePath(filePath)
{
    m_impl = new Impl();
    File f(filePath);
    if (f.open(IODevice::ReadOnly)) {
        m_impl->data = f.readAll();
        m_impl->ba = m_impl->data.toQByteArrayNoCopy();
    }
    m_impl->buf.setBuffer(&m_impl->ba);
    m_impl->buf.open(QIODevice::ReadOnly);
    m_impl->zip = new MQZipReader(&m_impl->buf);
}

ZipReader::ZipReader(IODevice* device)
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

bool ZipReader::exists() const
{
    return File::exists(m_filePath);
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

ByteArray ZipReader::fileData(const std::string& fileName) const
{
    QByteArray ba = m_impl->zip->fileData(QString::fromStdString(fileName));
    return ByteArray(reinterpret_cast<const uint8_t*>(ba.constData()), ba.size());
}
