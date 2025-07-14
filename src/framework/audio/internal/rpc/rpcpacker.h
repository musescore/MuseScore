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

#include "global/types/bytearray.h"
#include "audiotypes.h"

namespace muse::audio::rpc {
class RpcPacker
{
public:
    RpcPacker() = default;

    static ByteArray pack(const bool& in);
    static bool unpack(const ByteArray& data, bool& out);

    static ByteArray pack(const TrackSequenceId& in);
    static bool unpack(const ByteArray& data, TrackSequenceId& out);
    static ByteArray pack(const TrackSequenceIdList& in);
    static bool unpack(const ByteArray& data, TrackSequenceIdList& out);

    static ByteArray pack(const RetVal<TrackIdList>& rv);
    static bool unpack(const ByteArray& data, RetVal<TrackIdList>& out);
};
}
