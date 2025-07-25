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
#pragma once

#include <string_view>

#include "../io/iodevice.h"
#include "../types/bytearray.h"
#include "../types/string.h"

#ifndef NO_QT_SUPPORT
#include <QString>
#endif

namespace muse {
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
    TextStream& operator<<(unsigned int val);
    TextStream& operator<<(double val);
    TextStream& operator<<(signed long int val);
    TextStream& operator<<(unsigned long int val);
    TextStream& operator<<(signed long long val);
    TextStream& operator<<(unsigned long long val);
    TextStream& operator<<(const char* s);
    TextStream& operator<<(std::string_view);
    TextStream& operator<<(const ByteArray& b);
    TextStream& operator<<(const String& s);

#ifndef NO_QT_SUPPORT
    TextStream& operator<<(const QString& s);
#endif

private:
    void write(const char* ch, size_t len);
    io::IODevice* m_device = nullptr;
    ByteArray m_buf;
};
}
