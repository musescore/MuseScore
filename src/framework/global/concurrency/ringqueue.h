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

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

#include <vector>
#include <atomic>

namespace muse {
//! NOTE Single Producer/Single Consumer
template<typename T>
class RingQueue
{
private:

    struct ProducerSide {
        alignas(CACHE_LINE_SIZE) std::atomic<size_t> write_pos = 0;
        char padding[CACHE_LINE_SIZE - sizeof(std::atomic<size_t>) * 2];
    };

    struct ConsumerSide {
        alignas(CACHE_LINE_SIZE) std::atomic<size_t> read_pos = 0;
        char padding[CACHE_LINE_SIZE - sizeof(std::atomic<size_t>) * 2];
    };

    size_t m_capacity = 0;
    size_t m_mask = 0;
    std::vector<T> m_data;

    ProducerSide m_producer;
    ConsumerSide m_consumer;

public:
    explicit RingQueue(size_t initialCapacity)
        : m_capacity(nextPowerOfTwo(initialCapacity))
        , m_mask(m_capacity - 1)
        , m_data(m_capacity)
    {
    }

    ~RingQueue() = default;

    RingQueue(const RingQueue&) = delete;
    RingQueue& operator=(const RingQueue&) = delete;

    // Producer
    bool tryPush(const T& item)
    {
        return tryPush_impl(item);
    }

    bool tryPush(T&& item)
    {
        return tryPush_impl(std::move(item));
    }

    size_t availableWrite() const
    {
        const size_t write_idx = m_producer.write_pos.load(std::memory_order_relaxed);
        const size_t read_idx = m_consumer.read_pos.load(std::memory_order_acquire);
        return m_capacity - (write_idx - read_idx);
    }

    // Consumer
    bool tryPop(T& item)
    {
        const size_t read_idx = m_consumer.read_pos.load(std::memory_order_relaxed);
        const size_t write_idx = m_producer.write_pos.load(std::memory_order_acquire);

        if (read_idx >= write_idx) {
            return false;
        }

        item = std::move(m_data[read_idx & m_mask]);

        m_consumer.read_pos.store(read_idx + 1, std::memory_order_release);

        return true;
    }

    bool tryPopAll(std::vector<T>& out)
    {
        const size_t read_idx = m_consumer.read_pos.load(std::memory_order_relaxed);
        const size_t write_idx = m_producer.write_pos.load(std::memory_order_acquire);

        const size_t to_read = write_idx - read_idx;

        if (to_read > 0) {
            const size_t pos = read_idx & m_mask;
            const size_t first_chunk = std::min(to_read, m_capacity - pos);

            out.resize(to_read);

            std::move(m_data.begin() + pos,
                      m_data.begin() + pos + first_chunk,
                      out.begin());

            if (first_chunk < to_read) {
                std::move(m_data.begin(),
                          m_data.begin() + (to_read - first_chunk),
                          out.begin() + first_chunk);
            }

            m_consumer.read_pos.store(read_idx + to_read, std::memory_order_release);
        }

        return to_read;
    }

    size_t availableRead() const
    {
        const size_t write_idx = m_producer.write_pos.load(std::memory_order_acquire);
        const size_t read_idx = m_consumer.read_pos.load(std::memory_order_relaxed);
        return write_idx - read_idx;
    }

    // Service
    bool empty() const { return availableRead() == 0; }
    bool full() const { return availableWrite() == 0; }
    size_t capacity() const { return m_capacity; }

private:
    template<typename U>
    bool tryPush_impl(U&& item)
    {
        const size_t write_idx = m_producer.write_pos.load(std::memory_order_relaxed);
        const size_t read_idx = m_consumer.read_pos.load(std::memory_order_acquire);

        if (write_idx - read_idx >= m_capacity) {
            return false;
        }

        m_data[write_idx & m_mask] = std::forward<U>(item);
        m_producer.write_pos.store(write_idx + 1, std::memory_order_release);
        return true;
    }

    static size_t nextPowerOfTwo(size_t n)
    {
        if (n == 0) {
            return 1;
        }
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n |= n >> 32;
        return n + 1;
    }
};
}
