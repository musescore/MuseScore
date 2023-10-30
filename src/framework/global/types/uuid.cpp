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
#include "uuid.h"

#include "thirdparty/stduuid/stduuid/include/uuid.h"

using namespace mu;

String Uuid::createUuid()
{
    std::random_device randDevice;
    auto seedData = std::array<int, std::mt19937::state_size> {};
    std::generate(std::begin(seedData), std::end(seedData), std::ref(randDevice));
    std::seed_seq seq(std::begin(seedData), std::end(seedData));
    std::mt19937 generator(seq);
    uuids::uuid_random_generator gen(generator);

    uuids::uuid const id = gen();

    return String::fromStdString(uuids::to_string(id));
}
