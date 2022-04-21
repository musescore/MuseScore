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
#include <iostream>

#include <QList>
#include <vector>

template<typename T>
inline T value(const std::vector<T>& vec, size_t idx)
{
    if (idx < vec.size()) {
        return vec.at(idx);
    }

    if constexpr (std::is_pointer<T>::value) {
        return nullptr;
    }

    return T();
}

void print(const std::vector<int>& vec)
{
    for (int v : vec) {
        std::cout << v << " ";
    }
    std::cout << std::endl;
}

int main()
{
    std::cout << "Hello World" << std::endl;

    std::vector<int> l1 = { 1, 2, 3 };
    std::vector<int> l2 = { 11, 21, 31 };

    using StaffIdx = size_t;

    StaffIdx id = 2;
    if (id < l1.size()) {
        int v = l1.at(id);
        std::cout << "v: " << v << std::endl;
    }

    std::cout << " Goodbye!" << std::endl;

    return 0;
}
