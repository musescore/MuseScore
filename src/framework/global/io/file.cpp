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
#include "file.h"

#include <cstring>

using namespace mu::io;

File::File(const path& filePath)
    : m_filePath(filePath)
{
}

path File::filePath() const
{
    return m_filePath;
}

bool File::fetchData()
{
    RetVal<QByteArray> data = fileSystem()->readFile(m_filePath);
    if (!data.ret) {
        return false;
    }

    m_data = ByteArray(reinterpret_cast<const uint8_t*>(data.val.constData()), data.val.size());
    return true;
}

size_t File::dataSize() const
{
    return m_data.size();
}

const uint8_t* File::rawData() const
{
    return m_data.constData();
}

bool File::resizeData(size_t size)
{
    m_data.resize(m_data.size() + size);
    return true;
}

size_t File::writeData(const uint8_t* data, size_t len)
{
    std::memcpy(m_data.data() + pos(), data, len);
    Ret ret = fileSystem()->writeToFile(m_filePath, QByteArray(reinterpret_cast<const char*>(m_data.data()), m_data.size()));
    if (ret) {
        return len;
    }
    return 0;
}
