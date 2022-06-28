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
#include "zipwriter.h"

#include <QBuffer>

#include "io/file.h"
#include "internal/qzipwriter_p.h"

#include "log.h"

using namespace mu;

struct ZipWriter::Impl
{
    MQZipWriter* zip = nullptr;
    QByteArray data;
    QBuffer buf;
    bool isClosed = false;
};

ZipWriter::ZipWriter(const io::path_t& filePath)
{
    m_selfDevice = true;
    m_device = new io::File(filePath);
    if (!m_device->open(io::IODevice::WriteOnly)) {
        LOGE() << "failed open file: " << filePath;
    }

    m_impl = new Impl();
    m_impl->buf.setBuffer(&m_impl->data);
    m_impl->buf.open(QIODevice::WriteOnly);
    m_impl->zip = new MQZipWriter(&m_impl->buf);
}

ZipWriter::ZipWriter(io::IODevice* device)
{
    m_device = device;
    m_impl = new Impl();
    m_impl->buf.setBuffer(&m_impl->data);
    m_impl->buf.open(QIODevice::WriteOnly);
    m_impl->zip = new MQZipWriter(&m_impl->buf);
}

ZipWriter::~ZipWriter()
{
    close();
    delete m_impl->zip;
    delete m_impl;

    if (m_selfDevice) {
        m_device->close();
        delete m_device;
    }
}

void ZipWriter::flush()
{
    if (m_device) {
        m_device->seek(0);
        m_device->write(m_impl->data);
    }
}

void ZipWriter::close()
{
    if (m_impl->isClosed) {
        return;
    }

    m_impl->zip->close();
    if (m_device) {
        flush();
        m_device->close();
    }

    m_impl->isClosed = true;
}

ZipWriter::Status ZipWriter::status() const
{
    return static_cast<Status>(m_impl->zip->status());
}

void ZipWriter::addFile(const std::string& fileName, const ByteArray& data)
{
    m_impl->zip->addFile(QString::fromStdString(fileName), data.toQByteArrayNoCopy());
    flush();
}
