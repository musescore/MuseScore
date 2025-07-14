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

#include "../thirdparty/cppack/msgpack.hpp"
#include "../types/bytearray.h"

namespace muse {
class MsgPack
{
public:
    MsgPack() = default;

    template<class ... Types>
    static ByteArray pack(const Types&... args)
    {
        msgpack::Packer p;
        p.process(args ...);
        const std::vector<uint8_t>& d = p.vector();
        return ByteArray(&d[0], d.size());
    }

    template<class ... Types>
    static bool unpack(const ByteArray& data, Types&... args)
    {
        auto unpacker = msgpack::Unpacker(data.constData(), data.size());
        unpacker.process(args ...);
        return unpacker.ec != msgpack::UnpackerError::OutOfRange;
    }
};
}
