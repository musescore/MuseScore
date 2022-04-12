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

std::vector<int> mid(const std::vector<int>& c, size_t pos, int alength = -1)
{
    if (c.empty()) {
        return std::vector<int>();
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
        return std::vector<int>();
    }

    if (pos >= end) {
        return std::vector<int>();
    }

    std::vector<int> sub(c.begin() + pos, c.begin() + end);
    return sub;
}

static const QList<int> qlist = { 1, 2, 3, 4, 5, 6 };
static const std::vector<int> vlist = { 1, 2, 3, 4, 5, 6 };

void check(int pos, int len)
{
    std::cout << "=====================" << std::endl;
    std::cout << "pos: " << pos << ", len: " << len << std::endl;
    QList<int> qmid = qlist.mid(pos, len);
    for (int v : qmid) {
        std::cout << v << ", ";
    }
    std::cout << std::endl;

    std::vector<int> vmid = mid(vlist, pos, len);
    for (int v : vmid) {
        std::cout << v << ", ";
    }
    std::cout << std::endl;
}

template<typename Container, typename K, typename V>
inline V value(const Container& c, const K& k, const V& def)
{
    // vector
    if constexpr (std::is_same<V, typename Container::value_type>::value) {
        size_t idx = static_cast<size_t>(k);
        if (idx < c.size()) {
            return c.at(idx);
        }
    }
    // map
    else {
        auto it = c.find(k);
        if (it != c.end()) {
            return it->second;
        }
    }

    return def;
}

int main()
{
    std::cout << "Hello World" << std::endl;

//    check(3, -1);
//    check(0, -1);
//    check(3, 5);
//    check(3, 15);
//    check(14, 1);

    std::vector vec1 = { 1, 2, 3, 4, 5 };

    std::cout << "size: " << vec1.size() << std::endl;
    vec1.erase(vec1.begin() + 1);

    for (int v : vec1) {
        std::cout << v << ", ";
    }
    std::cout << std::endl;
    std::cout << "size: " << vec1.size() << std::endl;

    std::cout << " Goodbye!" << std::endl;

    return 0;
}
