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

#include "internal/qzipwriter_p.h"

using namespace mu;

struct ZipWriter::Impl
{
    MQZipWriter* zip = nullptr;
    QBuffer buf;
    bool isClosed = false;
};

ZipWriter::ZipWriter(QIODevice* device)
{
    m_impl = new Impl();
    m_impl->zip = new MQZipWriter(device);
}

ZipWriter::ZipWriter(io::IODevice* device)
{
    m_device = device;
    m_impl = new Impl();
    m_impl->zip = new MQZipWriter(&m_impl->buf);
}

ZipWriter::~ZipWriter()
{
    delete m_impl->zip;
    delete m_impl;
}

void ZipWriter::close()
{
    if (m_impl->isClosed) {
        return;
    }
    m_impl->isClosed = true;

    m_impl->zip->close();
    if (m_device) {
        QByteArray data = m_impl->buf.readAll();
        m_device->write(data);
        m_device->close();
    }
}

ZipWriter::Status ZipWriter::status() const
{
    return static_cast<Status>(m_impl->zip->status());
}

void ZipWriter::addFile(const QString& fileName, const QByteArray& data)
{
    m_impl->zip->addFile(fileName, data);
}
