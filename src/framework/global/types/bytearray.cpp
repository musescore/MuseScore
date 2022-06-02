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
#include "types/bytearray.h"

#include <cstring>
#include <cassert>

using namespace mu;

ByteArray::ByteArray()
    : m_size(0)
{
}

ByteArray::ByteArray(const uint8_t* data, size_t size)
    : m_size(size)
{
    m_data.reset(new uint8_t[m_size + 1]);
    m_data.get()[m_size] = 0;
    std::memcpy(m_data.get(), data, m_size);
}

ByteArray::ByteArray(const char* str)
{
    m_size = std::strlen(str);
    m_data.reset(new uint8_t[m_size + 1]);
    m_data.get()[m_size] = 0;
    std::memcpy(m_data.get(), str, m_size);
}

ByteArray::ByteArray(size_t size)
    : m_size(size)
{
    m_data.reset(new uint8_t[m_size + 1]);
    m_data.get()[m_size] = 0;
}

ByteArray ByteArray::fromRawData(const uint8_t* data, size_t size)
{
    ByteArray ba;
    ba.m_size = size;
    ba.m_rawData = data;
    return ba;
}

void ByteArray::detachRawDataIfNeed()
{
    if (m_rawData) {
        m_data.reset(new uint8_t[m_size + 1]);
        m_data.get()[m_size] = 0;
        std::memcpy(m_data.get(), m_rawData, m_size);
        m_rawData = nullptr;
    }
}

bool ByteArray::operator==(const ByteArray& other) const
{
    if (m_size != other.m_size) {
        return false;
    }
    return std::memcmp(constData(), other.constData(), m_size) == 0;
}

uint8_t* ByteArray::data()
{
    detachRawDataIfNeed();
    return m_data.get();
}

const uint8_t* ByteArray::constData() const
{
    if (m_rawData) {
        return m_rawData;
    }

    return m_data.get();
}

size_t ByteArray::size() const
{
    return m_size;
}

bool ByteArray::empty() const
{
    return size() == 0;
}

void ByteArray::resize(size_t nsize)
{
    if (nsize == m_size) {
        return;
    }

    detachRawDataIfNeed();

    uint8_t* nbyte = new uint8_t[nsize + 1];
    nbyte[nsize] = 0;
    if (m_data) {
        size_t min = nsize < m_size ? nsize : m_size;
        std::memcpy(nbyte, m_data.get(), min);
    }
    m_size = nsize;
    m_data.reset(nbyte);
}

void ByteArray::truncate(size_t pos)
{
    if (pos >= m_size) {
        return;
    }
    resize(pos);
}

void ByteArray::clear()
{
    resize(0);
}

ByteArray& ByteArray::insert(size_t pos, uint8_t b)
{
    if (pos > m_size) {
        return *this;
    }

    detachRawDataIfNeed();

    if (!m_data && pos == 0) {
        if (m_buffer_size == 0) {
            m_buffer_size = 128;
            m_data.reset(new uint8_t[m_buffer_size]);
        }
        m_size = 1;
        m_data.get()[0] = b;
        m_data.get()[1] = 0;
        return *this;
    }

    if (m_size + 2 > m_buffer_size) {
        m_buffer_size *= 2;
        uint8_t* ndata = new uint8_t[m_buffer_size];
        std::memcpy(ndata, m_data.get(), m_size + 1);
        m_data.reset(ndata);
    }

    if (pos != m_size) {
        std::memmove(&m_data.get()[pos + 1], &m_data.get()[pos], m_size - pos + 1);
        m_data.get()[pos] = b;
    } else {
        m_data.get()[pos] = b;
        m_data.get()[pos + 1] = 0;
    }

    m_size += 1;
    return *this;
}

void ByteArray::push_back(uint8_t b)
{
    insert(m_size, b);
}

void ByteArray::push_back(const uint8_t* b, size_t len)
{
    detachRawDataIfNeed();

    uint8_t* nbyte = new uint8_t[m_size + len + 1];
    nbyte[m_size + len] = 0;
    std::memcpy(nbyte, m_data.get(), m_size);
    std::memcpy(&nbyte[m_size], b, len);
    m_data.reset(nbyte);
    m_size += len;
}

void ByteArray::push_back(const ByteArray& ba)
{
    push_back(ba.constData(), ba.size());
}

uint8_t ByteArray::operator[](size_t pos) const
{
    assert(pos < m_size);
    if (pos < m_size) {
        return constData()[pos];
    }
    return 0;
}

uint8_t& ByteArray::operator[](size_t pos)
{
    detachRawDataIfNeed();

    assert(pos < m_size);
    if (pos < m_size) {
        return m_data.get()[pos];
    }

    static uint8_t dummy;
    return dummy;
}

ByteArray ByteArray::left(size_t len) const
{
    return ByteArray(m_data.get(), len);
}

ByteArray ByteArray::right(size_t len) const
{
    return ByteArray(&(m_data.get()[m_size - len]), len);
}
