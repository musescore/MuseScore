/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include <vector>
#include <cassert>

namespace muse::audio {
//! NOTE This is a ring buffer, not thread-safe.
//! Designed to align processed audio blocks
//! For example, we expect block 512, but the driver requests 470
class AlignmentBuffer
{
public:
    AlignmentBuffer(size_t capacity)
        : m_buf(capacity) {}

    inline size_t capacity() const
    {
        return m_buf.size();
    }

    inline size_t availableRead() const
    {
        return m_availableRead;
    }

    inline size_t availableWrite() const
    {
        return m_buf.size() - m_availableRead;
    }

    bool write(const float* data, size_t size)
    {
        assert(size <= availableWrite());
        if (!(size <= availableWrite())) {
            return false;
        }

        if (m_tail + size <= m_buf.size()) {
            std::copy(data, data + size, m_buf.begin() + m_tail);
            m_tail += size;
        } else {
            size_t firstPart = m_buf.size() - m_tail;
            std::copy(data, data + firstPart, m_buf.begin() + m_tail);
            std::copy(data + firstPart, data + size, m_buf.begin());
            m_tail = size - firstPart;
        }

        m_availableRead += size;
        return true;
    }

    bool read(float* dest, size_t size)
    {
        if (size == 0) {
            return true;
        }

        assert(size <= m_availableRead);
        if (!(size <= m_availableRead)) {
            return false;
        }

        if (m_head + size <= m_buf.size()) {
            std::copy(m_buf.begin() + m_head, m_buf.begin() + m_head + size, dest);
            m_head += size;
        } else {
            size_t firstPart = m_buf.size() - m_head;
            std::copy(m_buf.begin() + m_head, m_buf.begin() + m_buf.size(), dest);
            std::copy(m_buf.begin(), m_buf.begin() + (size - firstPart), dest + firstPart);
            m_head = size - firstPart;
        }

        m_availableRead -= size;
        return true;
    }

private:
    std::vector<float> m_buf;
    size_t m_availableRead = 0;
    size_t m_head = 0;
    size_t m_tail = 0;
};
}
