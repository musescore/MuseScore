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
#ifndef MU_IO_BYTEARRAY_H
#define MU_IO_BYTEARRAY_H

#include <cstdint>
#include <memory>

namespace mu::io {
class ByteArray
{
public:
    ByteArray();
    ByteArray(const uint8_t* data);
    ByteArray(const uint8_t* data, size_t size);

    bool operator==(const ByteArray& other) const;
    bool operator!=(const ByteArray& other) const { return !operator==(other); }

    uint8_t* data();
    const uint8_t* data() const;
    size_t size() const;
    bool empty() const;

    ByteArray& insert(size_t pos, uint8_t b);
    void push_back(uint8_t b);
    void push_back(const ByteArray& other);
    uint8_t operator[](size_t pos) const;
    uint8_t& operator[](size_t pos);

    void resize(size_t nsize);
    void truncate(size_t pos);

    ByteArray left(size_t len) const;
    ByteArray right(size_t len) const;

private:

    std::shared_ptr<uint8_t> m_data;
    size_t m_size = 0;
    size_t m_buffer_size = 0;
};
}

#endif // MU_IO_BYTEARRAY_H
