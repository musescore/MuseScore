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
#include "rpcpacker.h"

#include "global/serialization/msgpack.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::rpc;

ByteArray RpcPacker::pack(const bool& in)
{
    return MsgPack::pack(in);
}

bool RpcPacker::unpack(const ByteArray& data, bool& out)
{
    return MsgPack::unpack(data, out);
}

ByteArray RpcPacker::pack(const TrackSequenceId& in)
{
    return MsgPack::pack(in);
}

bool RpcPacker::unpack(const ByteArray& data, TrackSequenceId& out)
{
    return MsgPack::unpack(data, out);
}

ByteArray RpcPacker::pack(const TrackSequenceIdList& in)
{
    return MsgPack::pack(in);
}

bool RpcPacker::unpack(const ByteArray& data, TrackSequenceIdList& out)
{
    return MsgPack::unpack(data, out);
}

ByteArray RpcPacker::pack(const RetVal<TrackIdList>& rv)
{
    return MsgPack::pack(rv.ret.code(), rv.ret.text(), rv.val);
}

bool RpcPacker::unpack(const ByteArray& data, RetVal<TrackIdList>& out)
{
    int code = -1;
    std::string text;
    bool ok = MsgPack::unpack(data, code, text, out.val);
    out.ret = Ret(code, text);
    return ok;
}
