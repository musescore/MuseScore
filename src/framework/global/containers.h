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

//! NOTE useful functions for containers

namespace mu {
template<typename Container, typename T>
inline bool contains(const Container& c, const T& v)
{
    return std::find(c.cbegin(), c.cend(), v) != c.cend();
}

template<typename Container, typename T>
inline bool remove(Container& c, const T& v)
{
    return c.erase(std::remove(c.begin(), c.end(), v), c.end()) != c.end();
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
}

#endif // MU_GLOBAL_CONTAINERS_H
