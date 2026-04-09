/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#include "filestream.h"

#include "ioretcodes.h"

#include "log.h"

using namespace muse;
using namespace muse::io;

FileStream::FileStream(const path_t& filePath)
    : m_filePath(filePath)
{
}

FileStream::~FileStream()
{
    if (m_streamId != INVALID_STREAM_ID) {
        fileSystem()->closeStream(m_streamId);
    }
}

const path_t& FileStream::filePath() const
{
    return m_filePath;
}

bool FileStream::doOpen(OpenMode m)
{
    if (m_streamId != INVALID_STREAM_ID) {
        fileSystem()->closeStream(m_streamId);
        m_streamId = INVALID_STREAM_ID;
    }

    io::OpenMode fsMode;
    switch (m) {
    case OpenMode::WriteOnly:
        fsMode = io::OpenMode::WriteOnly;
        m_size = 0;
        break;
    case OpenMode::Append: {
        fsMode = io::OpenMode::Append;
        RetVal<uint64_t> sz = fileSystem()->fileSize(m_filePath);
        m_size = sz.ret ? static_cast<size_t>(sz.val) : 0;
        break;
    }
    default:
        setError(int(Err::FSWriteError), "FileStream supports only WriteOnly and Append modes");
        return false;
    }

    RetVal<StreamId> rv = fileSystem()->openStream(m_filePath, fsMode);
    if (!rv.ret) {
        setError(rv.ret.code(), rv.ret.text());
        return false;
    }

    m_streamId = rv.val;
    m_expectedPos = m_size;
    return true;
}

size_t FileStream::dataSize() const
{
    return m_size;
}

const uint8_t* FileStream::rawData() const
{
    return nullptr;
}

bool FileStream::resizeData(size_t size)
{
    m_size = size;
    return true;
}

size_t FileStream::writeData(const uint8_t* data, size_t len)
{
    IF_ASSERT_FAILED(m_streamId != INVALID_STREAM_ID) {
        return 0;
    }

    uint64_t offset = (pos() == m_expectedPos) ? STREAM_POS_CURRENT : static_cast<uint64_t>(pos());

    Ret ret = fileSystem()->writeToStream(m_streamId, ByteArray::fromRawData(data, len), offset);
    if (!ret) {
        setError(ret.code(), ret.text());
        return 0;
    }

    m_expectedPos = pos() + len;
    return len;
}
