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

int main()
{
    std::cout << "Hello World" << std::endl;

    std::list<int> vec1 = { 1, 2, 3, 4, 5 };
    std::list<int> vec2 = { 1, 2, 3 };

    int v = *vec2.end();
    std::cout << "v: " << v << std::endl;

    if (std::equal(vec1.begin(), vec1.end(), vec2.begin())) {
        std::cout << " equal" << std::endl;
    }

    std::cout << " Goodbye!" << std::endl;

    return 0;
}
