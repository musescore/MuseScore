/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include <string>

namespace muse {
class ByteArray;

namespace io {
class IODevice;
}

class ZipWriter
{
public:
    explicit ZipWriter(io::IODevice* device);
    ~ZipWriter();

    void close();
    bool hasError() const;

    void addFile(const std::string& fileName, const ByteArray& data);

private:

    void flush();

    struct Impl;
    Impl* m_impl = nullptr;
    io::IODevice* m_device = nullptr;
};
}
