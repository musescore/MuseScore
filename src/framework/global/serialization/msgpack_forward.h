/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#pragma once

#include <vector>
#include <cstdint>

namespace kors::msgpack {
template<typename Data>
class DataPacker;
template<typename Data>
class DataUnPacker;
struct Cursor;
}

namespace muse::msgpack {
template<typename Data = std::vector<uint8_t>>
using DataPacker = kors::msgpack::DataPacker<Data>;
using Packer = kors::msgpack::DataPacker<std::vector<uint8_t>>;
template<typename Data = std::vector<uint8_t>>
using DataUnPacker = kors::msgpack::DataUnPacker<Data>;
using UnPacker = kors::msgpack::DataUnPacker<std::vector<uint8_t>>;
using Cursor = kors::msgpack::Cursor;
}
