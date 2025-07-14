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
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "serialization/msgpack.h"
#include "types/bytearray.h"

using namespace muse;

class Global_Ser_MsgPack : public ::testing::Test
{
public:
};

struct Object {
    uint64_t ui64 = 0;
    double real = 0.0;
    std::string str;
    bool b = false;
    std::vector<int> vi;
    std::map<std::string, int> map;

    // origin way
    template<class T>
    void pack(T& pack)
    {
        pack(ui64, real, str, b, vi, map);
    }
};

TEST_F(Global_Ser_MsgPack, WriteRead)
{
    ByteArray data;

    // Write
    {
        Object obj;
        obj.ui64 = 42;
        obj.real = 4.2;
        obj.str = "42";
        obj.vi = { 4, 2 };
        obj.b = true;
        obj.map = { { "42", 42 } };

        // origin
        std::vector<uint8_t> vd = msgpack::pack(obj);

        // muse wrap
        data = MsgPack::pack(obj.ui64, obj.real, obj.str, obj.b, obj.vi, obj.map);

        EXPECT_TRUE(data == ByteArray::fromRawData(&vd[0], vd.size()));
    }

    // Read
    {
        // origin
        // Object obj = msgpack::unpack<Object>(data.constData(), data.size());

        // muse wrap
        Object obj;
        bool ok = MsgPack::unpack(data, obj.ui64, obj.real, obj.str, obj.b, obj.vi, obj.map);
        EXPECT_TRUE(ok);

        EXPECT_EQ(obj.ui64, 42);
        EXPECT_DOUBLE_EQ(obj.real, 4.2);
        EXPECT_EQ(obj.str, "42");
        EXPECT_EQ(obj.b, true);
        EXPECT_EQ(obj.vi.size(), 2);
        EXPECT_EQ(obj.vi.at(0), 4);
        EXPECT_EQ(obj.vi.at(1), 2);
        EXPECT_EQ(obj.map.size(), 1);
        EXPECT_EQ(obj.map["42"], 42);
    }
}
