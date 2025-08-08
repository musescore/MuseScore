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

namespace muse {
struct ObjectCustom {
    int32_t i32 = 0;
    std::string str;
};
}

#include "serialization/msgpack_forward.h"

void pack_custom(std::vector<uint8_t>& data, const muse::ObjectCustom& value);
bool unpack_custom(muse::msgpack::Cursor& cursor, muse::ObjectCustom& value);

#include "serialization/msgpack.h"

void pack_custom(std::vector<uint8_t>& data, const muse::ObjectCustom& value)
{
    muse::msgpack::pack_to_data(data, value.i32, value.str);
}

bool unpack_custom(muse::msgpack::Cursor& cursor, muse::ObjectCustom& value)
{
    return muse::msgpack::unpack(cursor, value.i32, value.str);
}

using namespace muse;

class Global_Ser_Msgpack : public ::testing::Test
{
public:
};

struct ObjectWithPack {
    int32_t i32 = 0;
    uint64_t ui64 = 0;
    float float32 = 0.0;
    double float64 = 0.0;
    std::string str;
    bool b = false;
    std::vector<int> vi;
    std::map<std::string, int> map;
    String mustr;
    number_t<float> mureal;

    template<typename T>
    void pack(T& pack)
    {
        pack(i32, ui64, float32, float64, str, b, vi, map, mustr, mureal);
    }
};

TEST_F(Global_Ser_Msgpack, String)
{
    String str1 = u"Hello World!";

    ByteArray data = msgpack::pack(str1);

    String str2;

    bool ok = msgpack::unpack(data, str2);

    EXPECT_TRUE(ok);
    EXPECT_EQ(str1, str2);
}

TEST_F(Global_Ser_Msgpack, Number)
{
    number_t<float> num1_1 = 0.34f;
    number_t<double> num1_2 = 46.0;
    number_t<int> num1_3 = 57;

    ByteArray data = msgpack::pack(num1_1, num1_2, num1_3);

    number_t<float> num2_1;
    number_t<double> num2_2;
    number_t<int> num2_3;

    bool ok = msgpack::unpack(data, num2_1, num2_2, num2_3);

    EXPECT_TRUE(ok);
    EXPECT_EQ(num1_1, num2_1);
    EXPECT_EQ(num1_2, num2_2);
    EXPECT_EQ(num1_3, num2_3);
}

TEST_F(Global_Ser_Msgpack, ObjectWithPack)
{
    ByteArray data;

    // Write
    {
        ObjectWithPack obj;
        obj.i32 = 12;
        obj.ui64 = 42;
        obj.float32 = 0.42f;
        obj.float64 = 42.42;
        obj.str = "42";
        obj.b = true;
        obj.vi = { 4, 2 };
        obj.map = { { "42", 42 } };
        obj.mustr = "mu 42";
        obj.mureal = 42.0;

        data = msgpack::pack(obj);
    }

    // Read
    {
        ObjectWithPack obj;
        bool ok = msgpack::unpack(data, obj);
        EXPECT_TRUE(ok);

        EXPECT_EQ(obj.i32, 12);
        EXPECT_EQ(obj.ui64, 42);
        EXPECT_FLOAT_EQ(obj.float32, 0.42f);
        EXPECT_DOUBLE_EQ(obj.float64, 42.42);
        EXPECT_EQ(obj.str, "42");
        EXPECT_EQ(obj.b, true);
        EXPECT_EQ(obj.vi.size(), 2);
        EXPECT_EQ(obj.vi.at(0), 4);
        EXPECT_EQ(obj.vi.at(1), 2);
        EXPECT_EQ(obj.map.size(), 1);
        EXPECT_EQ(obj.map["42"], 42);
        EXPECT_EQ(obj.mustr, "mu 42");
        EXPECT_FLOAT_EQ(obj.mureal, 42.0);
    }
}

TEST_F(Global_Ser_Msgpack, ObjectCustom)
{
    ByteArray data;

    // Write
    {
        ObjectCustom obj;
        obj.i32 = 12;
        obj.str = "ha ha ha";

        data = msgpack::pack(obj);
    }

    // Read
    {
        ObjectCustom obj;
        bool ok = msgpack::unpack(data, obj);
        EXPECT_TRUE(ok);

        EXPECT_EQ(obj.i32, 12);
        EXPECT_EQ(obj.str, "ha ha ha");
    }
}
