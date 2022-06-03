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
#ifndef MU_GLOBAL_BYTEARRAY_H
#define MU_GLOBAL_BYTEARRAY_H

#include <cstdint>
#include <memory>

//#ifndef NO_QT_SUPPORT
#include <QByteArray>
//#endif

namespace mu {
class ByteArray
{
public:
    ByteArray();
    ByteArray(const uint8_t* data, size_t size);
    ByteArray(const char* str);
    ByteArray(size_t size);

    //! NOTE Not coped!!!
    static ByteArray fromRawData(const uint8_t* data, size_t size);

    bool operator==(const ByteArray& other) const;
    bool operator!=(const ByteArray& other) const { return !operator==(other); }

    uint8_t* data();
    const uint8_t* constData() const;
    const char* constChar() const; // data as char*
    size_t size() const;
    bool empty() const;

    ByteArray& insert(size_t pos, uint8_t b);
    void push_back(uint8_t b);
    void push_back(const uint8_t* b, size_t len);
    void push_back(const ByteArray& ba);
    uint8_t operator[](size_t pos) const;
    uint8_t& operator[](size_t pos);

    void resize(size_t nsize);
    void truncate(size_t pos);
    void clear();

    ByteArray left(size_t len) const;
    ByteArray right(size_t len) const;

//#ifndef NO_QT_SUPPORT
    static ByteArray fromQByteArray(const QByteArray& ba)
    {
        return ByteArray(reinterpret_cast<const uint8_t*>(ba.constData()), ba.size());
    }

    static ByteArray fromQByteArrayNoCopy(const QByteArray& ba)
    {
        return fromRawData(reinterpret_cast<const uint8_t*>(ba.constData()), ba.size());
    }

    QByteArray toQByteArray() const
    {
        return QByteArray(reinterpret_cast<const char*>(constData()), static_cast<int>(size()));
    }

    QByteArray toQByteArrayNoCopy() const
    {
        return QByteArray::fromRawData(reinterpret_cast<const char*>(constData()), static_cast<int>(size()));
    }

//#endif

private:

    void detachRawDataIfNeed();

    std::shared_ptr<uint8_t> m_data;
    const uint8_t* m_rawData = nullptr;
    size_t m_size = 0;
    size_t m_buffer_size = 0;
};
}

#endif // MU_GLOBAL_BYTEARRAY_H
