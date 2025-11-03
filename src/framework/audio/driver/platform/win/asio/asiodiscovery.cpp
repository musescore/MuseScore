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
#include "asiodiscovery.h"

#include "ASIOSDK/host/asiodrivers.h"

#include "log.h"

using namespace muse::audio;

AsioDiscovery::AsioDiscovery()
{
}

std::vector<std::string> AsioDiscovery::driverNameList() const
{
    char names[16][32];
    char* pointers[16];

    for (int i = 0; i < 16; i++) {
        pointers[i] = names[i];
    }

    AsioDrivers asio;
    long count = asio.getDriverNames(pointers, 16);

    std::vector<std::string> result;
    result.reserve(count);
    for (long i = 0; i < count; i++) {
        result.push_back(names[i]);
    }

    return result;
}

void AsioDiscovery::dump()
{
    std::vector<std::string> names = driverNameList();

    std::string dump = "ASIO Drivers: \n";
    for (size_t i = 0; i < names.size(); ++i) {
        dump += std::to_string(i) + " " + names.at(i) + "\n";
    }

    LOGI() << dump;
}
