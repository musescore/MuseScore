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
#ifndef MU_GLOBAL_CONTAINERS_H
#define MU_GLOBAL_CONTAINERS_H

#include <algorithm>
#include <vector>

//! NOTE useful functions for containers

namespace mu {
template<typename Container, typename T>
inline bool contains(const Container& c, const T& v)
{
    // vector
    if constexpr (std::is_same<T, typename Container::value_type>::value) {
        return std::find(c.cbegin(), c.cend(), v) != c.cend();
    }
    // map
    else {
        return c.find(v) != c.cend();
    }
}

template<typename T>
inline T value(const std::vector<T>& vec, size_t idx)
{
    if (idx < vec.size()) {
        return vec.at(idx);
    }

    if (std::is_pointer<T>::value) {
        return nullptr;
    }

    return T();
}

template<typename T>
inline T take(std::vector<T>& vec, size_t idx)
{
    T v = value(vec, idx);
    vec.erase(vec.begin() + idx);
    return v;
}

template<typename T>
inline T takeFirst(std::vector<T>& vec)
{
    return take(vec, 0);
}

template<typename Container, typename T>
inline bool remove(Container& c, const T& v)
{
    // vector
    if constexpr (std::is_same<T, typename Container::value_type>::value) {
        return c.erase(std::remove(c.begin(), c.end(), v), c.end()) != c.end();
    }
    // map
    else {
        auto it = c.find(v);
        if (it != c.end()) {
            c.erase(it);
            return true;
        }
        return false;
    }
}

template<typename Container, typename T>
inline int indexOf(const Container& c, const T& v)
{
    auto it = std::find(c.cbegin(), c.cend(), v);
    if (it != c.cend()) {
        return std::distance(c.cbegin(), it);
    }
    return -1;
}

template<typename T>
std::vector<T> mid(const std::vector<T>& c, size_t pos, int alength = -1)
{
    if (c.empty()) {
        return std::vector<T>();
    }

    size_t end = 0;
    if (alength < 0) {
        end = c.size();
    } else {
        end = pos + static_cast<size_t>(alength);
    }

    if (end > (c.size())) {
        end = c.size();
    }

    if (end == 0) {
        return std::vector<T>();
    }

    if (pos >= end) {
        return std::vector<T>();
    }

    std::vector<T> sub(c.begin() + pos, c.begin() + end);
    return sub;
}

template<typename Map>
inline auto keys(const Map& m) -> std::vector<typename Map::key_type>
{
    std::vector<typename Map::key_type> result;
    for (auto&& p : m) {
        result.push_back(p.first);
    }
    return result;
}

template<typename Map, typename K>
inline auto value(const Map& m, const K& k) -> typename Map::mapped_type
{
    auto it = m.find(k);
    if (it != m.end()) {
        return it->second;
    }
    typename Map::mapped_type def {};
    return def;
}

template<typename Map, typename K, typename V>
inline auto value(const Map& m, const K& k, const V& def) -> typename Map::mapped_type
{
    auto it = m.find(k);
    if (it != m.end()) {
        return it->second;
    }
    return def;
}

template<typename Map, typename K>
inline auto take(Map& m, const K& k) -> typename Map::mapped_type
{
    auto it = m.find(k);
    if (it != m.end()) {
        auto v = it->second;
        m.erase(it);
        return v;
    }
    typename Map::mapped_type def {};
    return def;
}
}

#endif // MU_GLOBAL_CONTAINERS_H
