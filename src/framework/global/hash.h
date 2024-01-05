/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
#ifndef MUSE_GLOBAL_HASH_H
#define MUSE_GLOBAL_HASH_H

#include <functional>

namespace muse {
inline size_t hash(int a, int b, char c)
{
    size_t h1 = std::hash<int> {}(a);
    size_t h2 = std::hash<int> {}(b);
    size_t h3 = std::hash<int> {}(static_cast<int>(c));
    return h1 ^ (h2 << 1) ^ (h3 << 2);
}

inline size_t hash(int a, bool b, bool c)
{
    size_t h1 = std::hash<int> {}(a);
    size_t h2 = std::hash<int> {}(static_cast<int>(b));
    size_t h3 = std::hash<int> {}(static_cast<int>(c));
    return h1 ^ (h2 << 1) ^ (h3 << 2);
}
}
#endif // MUSE_GLOBAL_HASH_H
