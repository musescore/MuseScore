/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include <limits>

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

    int v1 = 100004321;
    float v2 = 100004321.;

    std::cout << "v1: " << static_cast<float>(v1) << "\n";
    std::cout << "v2: " << v2 << "\n";

    uint64_t vi = std::numeric_limits<uint64_t>::max();
    std::cout << "vi: " << static_cast<float>(vi - 10) << "\n";

    std::cout << "float: " << std::numeric_limits<float>::max() << "\n";
    std::cout << "int: " << std::numeric_limits<int>::max() << "\n";
    std::cout << "double: " << std::numeric_limits<double>::max() << "\n";
    std::cout << "uint64_t: " << std::numeric_limits<uint64_t>::max() << "\n";

    std::cout << " Goodbye!" << std::endl;

    return 0;
}
