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

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace muse {
template<typename T>
struct VectorView
{
public:
    VectorView(std::vector<T>& vec)
        : m_begin(vec.begin()), m_end(vec.end()) {}

    using iterator = typename std::vector<T>::iterator;
    using reverse_iterator = typename std::vector<T>::reverse_iterator;

    iterator begin() { return m_begin; }
    iterator end() { return m_end; }

    reverse_iterator rbegin() { return reverse_iterator { m_end }; }
    reverse_iterator rend() { return reverse_iterator { m_begin }; }

    bool empty() const { return m_begin == m_end; }

    T& front() const
    {
        if (empty()) {
            throw std::out_of_range("VectorView::front(): empty view");
        }
        return *m_begin;
    }

    T& pop_front()
    {
        if (empty()) {
            throw std::out_of_range("VectorView::pop_front(): empty view");
        }
        T& v = *m_begin;
        ++m_begin;
        return v;
    }

    T& back() const
    {
        if (empty()) {
            throw std::out_of_range("VectorView::back(): empty view");
        }
        return *(m_end - 1);
    }

    T& pop_back()
    {
        if (empty()) {
            throw std::out_of_range("VectorView::pop_back(): empty view");
        }
        --m_end;
        return *m_end;
    }

    bool remove(const T& v)
    {
        auto originalEnd = m_end;
        m_end = std::remove(m_begin, m_end, v);
        return m_end != originalEnd;
    }

    template<typename Predicate>
    bool remove_if(Predicate&& pred)
    {
        auto originalEnd = m_end;
        m_end = std::remove_if(m_begin, m_end, std::forward<Predicate>(pred));
        return m_end != originalEnd;
    }

    bool reverse_remove(const T& v)
    {
        auto originalBegin = m_begin;
        m_begin = std::remove(rbegin(), rend(), v).base();
        return m_begin != originalBegin;
    }

    template<typename Predicate>
    bool reverse_remove_if(Predicate&& pred)
    {
        auto originalBegin = m_begin;
        m_begin = std::remove_if(rbegin(), rend(), std::forward<Predicate>(pred)).base();
        return m_begin != originalBegin;
    }

private:
    iterator m_begin;
    iterator m_end;
};
}
