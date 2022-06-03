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
#ifndef MU_GLOBAL_TEXTSTREAM_H
#define MU_GLOBAL_TEXTSTREAM_H

#include <QString>
#include "io/iodevice.h"
#include "types/bytearray.h"
#include "types/string.h"

namespace mu {
class TextStream
{
public:
    TextStream() = default;
    explicit TextStream(io::IODevice* device);
    virtual ~TextStream();

    void setDevice(io::IODevice* device);

    void flush();

    TextStream& operator<<(char ch);
    TextStream& operator<<(int val);
    TextStream& operator<<(double val);
    TextStream& operator<<(int64_t val);
    TextStream& operator<<(size_t val);
    TextStream& operator<<(const char* s);
    TextStream& operator<<(const std::string& s);
    TextStream& operator<<(const QString& s);
    TextStream& operator<<(const ByteArray& b);
    TextStream& operator<<(const AsciiString& s);
    TextStream& operator<<(const String& s);

private:
    void write(const char* ch, size_t len);
    io::IODevice* m_device = nullptr;
    ByteArray m_buf;
};
}

#endif // MU_GLOBAL_TEXTSTREAM_H
