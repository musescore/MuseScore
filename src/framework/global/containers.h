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
#include <list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

//! NOTE useful functions for containers

namespace mu {
// vector
template<typename T>
inline bool contains(const std::vector<T>& vec, const T& v)
{
    return std::find(vec.cbegin(), vec.cend(), v) != vec.cend();
}

template<typename T>
inline T value(const std::vector<T>& vec, size_t idx)
{
    if (idx < vec.size()) {
        return vec.at(idx);
    }

    if constexpr (std::is_pointer<T>::value) {
        return nullptr;
    } else {
        return T();
    }
}

template<typename T>
inline bool remove(std::vector<T>& vec, const T& v)
{
    return vec.erase(std::remove(vec.begin(), vec.end(), v), vec.end()) != vec.end();
}

template<typename T, typename Predicate>
inline bool remove_if(std::vector<T>& vec, Predicate p)
{
    return vec.erase(std::remove_if(vec.begin(), vec.end(), p), vec.end()) != vec.end();
}

template<typename T>
inline T takeAt(std::vector<T>& vec, size_t idx)
{
    T v = value(vec, idx);
    vec.erase(vec.begin() + idx);
    return v;
}

template<typename T>
inline T takeFirst(std::vector<T>& vec)
{
    return takeAt(vec, 0);
}

template<typename T>
inline T takeLast(std::vector<T>& vec)
{
    return takeAt(vec, vec.size() - 1);
}

template<typename T>
inline void swapItemsAt(std::vector<T>& vec, size_t idx1, size_t idx2)
{
    T v1 = vec[idx1];
    vec[idx1] = vec[idx2];
    vec[idx2] = v1;
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

template<typename T>
inline void join(std::vector<T>& l1, const std::vector<T>& l2)
{
    l1.insert(l1.end(), l2.begin(), l2.end());
}

// list
template<typename T>
inline bool contains(const std::list<T>& l, const T& v)
{
    return std::find(l.cbegin(), l.cend(), v) != l.cend();
}

template<typename T>
inline void join(std::list<T>& l1, const std::list<T>& l2)
{
    l1.insert(l1.end(), l2.begin(), l2.end());
}

template<typename T>
inline bool remove(std::list<T>& l, const T& v)
{
    if (!contains(l, v)) {
        return false;
    }
    l.remove(v);
    return true;
}

template<typename T>
inline T takeFirst(std::list<T>& l)
{
    T v = l.front();
    l.pop_front();
    return v;
}

template<typename T>
inline T takeLast(std::list<T>& l)
{
    T v = l.back();
    l.pop_back();
    return v;
}

// set
template<typename T>
inline bool contains(const std::set<T>& s, const T& v)
{
    return s.find(v) != s.cend();
}

template<typename T>
inline bool contains(const std::unordered_set<T>& s, const T& v)
{
    return s.find(v) != s.cend();
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

template<typename K, typename V>
inline bool contains(const std::map<K, V>& m, const K& k)
{
    return m.find(k) != m.cend();
}

template<typename K, typename V>
inline bool contains(const std::unordered_map<K, V>& m, const K& k)
{
    return m.find(k) != m.cend();
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

template<typename K, typename V>
inline std::set<K> uniqueKeys(const std::multimap<K, V>& mm)
{
    std::set<K> keys;
    for (auto it = mm.begin(); it != mm.end(); ++it) {
        keys.insert(it->first);
    }
    return keys;
}
}

#endif // MU_GLOBAL_CONTAINERS_H
