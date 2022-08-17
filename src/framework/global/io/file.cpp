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

#include "ioretcodes.h"

using namespace mu::io;

File::File(const path_t& filePath)
    : m_filePath(filePath)
{
}

File::~File()
{
    close();
}

path_t File::filePath() const
{
    return m_filePath;
}

bool File::exists() const
{
    return fileSystem()->exists(m_filePath);
}

bool File::exists(const path_t& filePath)
{
    return fileSystem()->exists(filePath);
}

bool File::remove(const path_t& filePath)
{
    return fileSystem()->remove(filePath);
}

bool File::setPermissionsAllowedForAll(const path_t& filePath)
{
    return fileSystem()->setPermissionsAllowedForAll(filePath);
}

bool File::remove()
{
    return fileSystem()->remove(m_filePath);
}

bool File::doOpen(OpenMode m)
{
    if (m == IODevice::WriteOnly) {
        //! NOTE Not need fetch data, file will be truncated
        return true;
    }

    if (!exists()) {
        if (m == OpenMode::ReadOnly) {
            m_error = Error::ReadError;
            return false;
        } else {
            return true;
        }
    }

    m_data = ByteArray();
    bool ok = fileSystem()->readFile(m_filePath, m_data);
    if (!ok) {
        m_error = Error::ReadError;
        return false;
    }

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
    m_data.resize(size);
    return true;
}

size_t File::writeData(const uint8_t* data, size_t len)
{
    std::memcpy(m_data.data() + pos(), data, len);
    bool ok = fileSystem()->writeFile(m_filePath, m_data);
    if (!ok) {
        m_error = Error::WriteError;
        return 0;
    }
    return len;
}

File::Error File::error() const
{
    return m_error;
}

std::string File::errorString() const
{
    switch (m_error) {
    case Error::NoError: return "";
    case Error::ReadError: return "failed read";
    case Error::WriteError: return "failed write";
    }
    return "";
}
