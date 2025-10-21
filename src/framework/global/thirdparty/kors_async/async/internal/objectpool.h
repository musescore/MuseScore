/*
MIT License

Copyright (c) Igor Korsukov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include <vector>
#include <atomic>
#include <cassert>
#include <mutex>

namespace kors::async {
template<typename T>
class ObjectPool
{
private:
    std::atomic<size_t> m_count = 0;
    std::mutex m_mutex;
    std::vector<T> m_objs;

public:

    ObjectPool(size_t capacity)
        : m_objs(capacity, nullptr) {}

    ~ObjectPool()
    {
        clear();
    }

    size_t capacity() const { return m_objs.size(); }
    size_t count() const { return m_count; }
    T at(size_t i) const { return m_objs.at(i); }

    void clear()
    {
        for (size_t i = 0; i < m_objs.size(); ++i) {
            if constexpr (std::is_pointer_v<T>) {
                T o = m_objs.at(i);
                delete o;
            }

            m_objs[i] = T{};
        }
    }

    template<typename Find, typename Create>
    T tryGet(Find&& find, Create&& create)
    {
        size_t count = m_count.load();
        assert(count <= m_objs.size());
        for (size_t i = 0; i < count; ++i) {
            T obj = m_objs.at(i);
            assert(obj);
            if (obj && find(obj)) {
                return obj;
            }
        }

        // not found, we will create

        // the `m_objs` collection itself doesn't change;
        // we don't lock it, we only lock a slot in this collection.
        // therefore, we can iterate over this collection
        // in other threads without a lock.
        // `m_count` limits the number of iterations (only filled slots).
        std::scoped_lock lock(m_mutex);
        count = m_count.load();
        for (size_t i = count; i < m_objs.size(); ++i) {
            T obj = m_objs.at(i);
            if (!obj) {
                // found an empty slot
                obj = create();
                m_objs[i] = obj;
                ++m_count;
                return obj;
            }
        }

        return nullptr;
    }
};
}
