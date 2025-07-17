#include <iostream>
#include <cassert>
#include <vector>
#include <list>
#include <set>
#include <map>

#include "msgpack.h"

using namespace app::msgpack;

constexpr bool ALL_TEST = false;
constexpr bool NIL_TEST = false;
constexpr bool BOOL_TEST = false;
constexpr bool INT8_TEST = false;
constexpr bool INT16_TEST = false;
constexpr bool INT32_TEST = false;
constexpr bool INT64_TEST = false;
constexpr bool REAL_TEST = false;
constexpr bool STR_TEST = false;
constexpr bool BIN_TEST = false;
constexpr bool ARR_TEST = false;
constexpr bool MAP_TEST = false;
constexpr bool CUSTOM_TEST = true;

inline constexpr bool is_equal(float v1, float v2)
{
    constexpr float compare_float_epsilon = 1e-5f;
    return std::abs(v1 - v2) <= std::max(std::abs(v1), std::abs(v2)) * compare_float_epsilon;
}

inline constexpr bool is_equal(double v1, double v2)
{
    constexpr double compare_float_epsilon = 1e-9;
    return std::abs(v1 - v2) <= std::max(std::abs(v1), std::abs(v2)) * compare_float_epsilon;
}

struct ObjectWithPack {
    int i = 0;
    std::string str;

    template<typename T>
    void pack(T& p)
    {
        p(i, str);
    }

    bool operator==(const ObjectWithPack& o) const
    {
        return i == o.i && str == o.str;
    }
};

struct ObjectWithPackUnPack {
    int i = 0;
    std::string str;

    template<typename T>
    void pack(T& p) const
    {
        p(i, str);
    }

    template<typename T>
    void unpack(T& p)
    {
        p(i, str);
    }

    bool operator==(const ObjectWithPackUnPack& o) const
    {
        return i == o.i && str == o.str;
    }
};

struct ObjectWithLowLevel {
    int i = 0;
    std::string str;

    void pack(std::vector<uint8_t>& data) const
    {
        Packer::pack(data, i, str);
    }

    void unpack(Cursor& c)
    {
        UnPacker::unpack(c, i, str);
    }

    bool operator==(const ObjectWithPackUnPack& o) const
    {
        return i == o.i && str == o.str;
    }
};

struct ObjectCustomPackUnPack {
    int i = 0;
    std::string str;

    bool operator==(const ObjectCustomPackUnPack& o) const
    {
        return i == o.i && str == o.str;
    }
};

template<typename T>
void pack_custom(T& p, const ObjectCustomPackUnPack& o)
{
    p(o.i, o.str);
}

template<typename T>
void unpack_custom(T& p, ObjectCustomPackUnPack& o)
{
    p(o.i, o.str);
}

struct ObjectCustomLowLevel {
    int i = 0;
    std::string str;

    bool operator==(const ObjectCustomLowLevel& o) const
    {
        return i == o.i && str == o.str;
    }
};

void pack_custom(std::vector<uint8_t>& data, const ObjectCustomLowLevel& o)
{
    Packer::pack(data, o.i, o.str);
}

bool unpack_custom(Cursor& c, ObjectCustomLowLevel& o)
{
    return UnPacker::unpack(c, o.i, o.str);
}

int main(int argc, char* argv[])
{
    std::cout << "Hello World, I am MsgPack" << std::endl;

    // nil
    if (ALL_TEST || NIL_TEST) {
        std::vector<uint8_t> data;
        Packer::pack(data, nullptr);
        std::nullptr_t null;
        bool ok = UnPacker::unpack(data, null);

        assert(ok);
    }

    // bool
    if (ALL_TEST || BOOL_TEST) {
        std::vector<uint8_t> data;
        bool val1_t = true;
        bool val1_f = false;
        Packer::pack(data, val1_t, val1_f);

        bool val2_t = false;
        bool val2_f = true;
        bool ok = UnPacker::unpack(data, val2_t, val2_f);

        assert(ok);
        assert(val1_t == val2_t);
        assert(val1_f == val2_f);
    }

    // int8_t, uint8_t
    if (ALL_TEST || INT8_TEST) {
        std::vector<uint8_t> data;
        int8_t val1_1 = 126;    // positive fixint
        int8_t val1_2 = -5;     // negative fixint
        int8_t val1_3 = -42;    // regular int8_t
        uint8_t val1_4 = 124;   // positive fixint
        uint8_t val1_5 = 234;   // regular uint8_t
        Packer::pack(data, val1_1, val1_2, val1_3, val1_4, val1_5);

        int8_t val2_1 = 0;
        int8_t val2_2 = 0;
        int8_t val2_3 = 0;
        uint8_t val2_4 = 0;
        uint8_t val2_5 = 0;
        bool ok = UnPacker::unpack(data, val2_1, val2_2, val2_3, val2_4, val2_5);

        assert(ok);
        assert(val1_1 == val2_1);
        assert(val1_2 == val2_2);
        assert(val1_3 == val2_3);
        assert(val1_4 == val2_4);
        assert(val1_5 == val2_5);

        // unpack 8 to 16
        int16_t val3_1 = 0;
        int16_t val3_2 = 0;
        int16_t val3_3 = 0;
        uint16_t val3_4 = 0;
        uint16_t val3_5 = 0;
        ok = UnPacker::unpack(data, val3_1, val3_2, val3_3, val3_4, val3_5);

        assert(ok);
        assert(val1_1 == val3_1);
        assert(val1_2 == val3_2);
        assert(val1_3 == val3_3);
        assert(val1_4 == val3_4);
        assert(val1_5 == val3_5);
    }

    // int16_t, uint16_t
    if (ALL_TEST || INT16_TEST) {
        std::vector<uint8_t> data;
        int16_t val1_1 = 126;    // positive fixint
        int16_t val1_2 = -5;     // negative fixint
        int16_t val1_3 = 346;    // regular int16_t
        int16_t val1_4 = -42;    // regular int16_t
        uint16_t val1_5 = 124;   // positive fixint
        uint16_t val1_6 = 348;   // regular uint16_t
        Packer::pack(data, val1_1, val1_2, val1_3, val1_4, val1_5, val1_6);

        int16_t val2_1 = 0;
        int16_t val2_2 = 0;
        int16_t val2_3 = 0;
        int16_t val2_4 = 0;
        uint16_t val2_5 = 0;
        uint16_t val2_6 = 0;
        bool ok = UnPacker::unpack(data, val2_1, val2_2, val2_3, val2_4, val2_5, val2_6);

        assert(ok);
        assert(val1_1 == val2_1);
        assert(val1_2 == val2_2);
        assert(val1_3 == val2_3);
        assert(val1_4 == val2_4);
        assert(val1_5 == val2_5);
        assert(val1_6 == val2_6);
    }

    // int32_t, uint32_t
    if (ALL_TEST || INT32_TEST) {
        std::vector<uint8_t> data;
        int32_t val1_1 = 126;    // positive fixint
        int32_t val1_2 = -5;     // negative fixint
        int32_t val1_3 = 346;    // regular int32_t
        int32_t val1_4 = -42;    // regular int32_t
        uint32_t val1_5 = 124;   // positive fixint
        uint32_t val1_6 = 348;   // regular uint32_t
        Packer::pack(data, val1_1, val1_2, val1_3, val1_4, val1_5, val1_6);

        int32_t val2_1 = 0;
        int32_t val2_2 = 0;
        int32_t val2_3 = 0;
        int32_t val2_4 = 0;
        uint32_t val2_5 = 0;
        uint32_t val2_6 = 0;
        bool ok = UnPacker::unpack(data, val2_1, val2_2, val2_3, val2_4, val2_5, val2_6);

        assert(ok);
        assert(val1_1 == val2_1);
        assert(val1_2 == val2_2);
        assert(val1_3 == val2_3);
        assert(val1_4 == val2_4);
        assert(val1_5 == val2_5);
        assert(val1_6 == val2_6);
    }

    // int64_t, uint64_t
    if (ALL_TEST || INT64_TEST) {
        std::vector<uint8_t> data;
        int64_t val1_1 = 126;    // positive fixint
        int64_t val1_2 = -5;     // negative fixint
        int64_t val1_3 = 346;    // regular int64_t
        int64_t val1_4 = -42;    // regular int64_t
        uint64_t val1_5 = 124;   // positive fixint
        uint64_t val1_6 = 348;   // regular uint64_t
        Packer::pack(data, val1_1, val1_2, val1_3, val1_4, val1_5, val1_6);

        int64_t val2_1 = 0;
        int64_t val2_2 = 0;
        int64_t val2_3 = 0;
        int64_t val2_4 = 0;
        uint64_t val2_5 = 0;
        uint64_t val2_6 = 0;
        bool ok = UnPacker::unpack(data, val2_1, val2_2, val2_3, val2_4, val2_5, val2_6);

        assert(ok);
        assert(val1_1 == val2_1);
        assert(val1_2 == val2_2);
        assert(val1_3 == val2_3);
        assert(val1_4 == val2_4);
        assert(val1_5 == val2_5);
        assert(val1_6 == val2_6);
    }

    // float, double
    if (ALL_TEST || REAL_TEST) {
        std::vector<uint8_t> data;
        float val1_1 = 0.42f;
        float val1_2 = 42.0f;
        float val1_3 = -0.42f;
        float val1_4 = -42.0f;
        double val1_5 = 0.84;
        double val1_6 = 84.0;
        double val1_7 = -0.84;
        double val1_8 = -84.0;
        Packer::pack(data, val1_1, val1_2, val1_3, val1_4, val1_5, val1_6, val1_7, val1_8);

        float val2_1 = 0.0f;
        float val2_2 = 0.0f;
        float val2_3 = 0.0f;
        float val2_4 = 0.0f;
        double val2_5 = 0.0;
        double val2_6 = 0.0;
        double val2_7 = 0.0;
        double val2_8 = 0.0;
        bool ok = UnPacker::unpack(data, val2_1, val2_2, val2_3, val2_4, val2_5, val2_6, val2_7, val2_8);

        assert(ok);
        assert(is_equal(val1_1, val2_1));
        assert(is_equal(val1_2, val2_2));
        assert(is_equal(val1_3, val2_3));
        assert(is_equal(val1_4, val2_4));
        assert(is_equal(val1_5, val2_5));
        assert(is_equal(val1_6, val2_6));
        assert(is_equal(val1_7, val2_7));
        assert(is_equal(val1_8, val2_8));
    }

    // string
    if (ALL_TEST || STR_TEST) {
        std::vector<uint8_t> data;

        std::string str1_fix = "Hello World!"; // size < 31
        std::string str1_8 = std::string(234, '1');
        std::string str1_16 = std::string(270, '2');
        std::string str1_32 = std::string(66535, '4');

        Packer::pack(data, str1_fix, str1_8, str1_16, str1_32);

        std::string str2_fix;
        std::string str2_8;
        std::string str2_16;
        std::string str2_32;

        bool ok = UnPacker::unpack(data, str2_fix, str2_8, str2_16, str2_32);

        assert(ok);
        assert(str1_fix == str2_fix);
        assert(str1_8 == str2_8);
        assert(str1_16 == str2_16);
        assert(str1_32 == str2_32);
    }

    // bin
    if (ALL_TEST || BIN_TEST) {
        std::vector<uint8_t> data;

        std::vector<uint8_t> v1_8 = std::vector<uint8_t>(234, 1);
        std::vector<uint8_t> v1_16 = std::vector<uint8_t>(270, 2);
        std::vector<uint8_t> v1_32 = std::vector<uint8_t>(66535, 4);

        Packer::pack(data, v1_8, v1_16, v1_32);

        std::vector<uint8_t> v2_8;
        std::vector<uint8_t> v2_16;
        std::vector<uint8_t> v2_32;

        bool ok = UnPacker::unpack(data, v2_8, v2_16, v2_32);

        assert(ok);
        assert(v1_8 == v2_8);
        assert(v1_16 == v2_16);
        assert(v1_32 == v2_32);
    }

    // array
    if (ALL_TEST || ARR_TEST) {
        std::vector<uint8_t> data;

        std::vector<int16_t> v1_8 = std::vector<int16_t>(12, 1);
        std::list<int16_t> v1_16 = std::list<int16_t>(270, 2);
        std::set<int16_t> v1_32;
        for (size_t i; i < 66535; ++i) {
            v1_32.insert(4);
        }

        Packer::pack(data, v1_8, v1_16, v1_32);

        std::vector<int16_t> v2_8;
        std::list<int16_t> v2_16;
        std::set<int16_t> v2_32;

        bool ok = UnPacker::unpack(data, v2_8, v2_16, v2_32);

        assert(ok);
        assert(v1_8 == v2_8);
        assert(v1_16 == v2_16);
        assert(v1_32 == v2_32);
    }

    // map
    if (ALL_TEST || MAP_TEST) {
        std::vector<uint8_t> data;

        std::map<int16_t, int32_t> v1_8;
        for (int16_t i; i < 12; ++i) {
            v1_8.insert({ i, 1 });
        }

        std::map<int16_t, int32_t> v1_16;
        for (int16_t i; i < 270; ++i) {
            v1_8.insert({ i, 1 });
        }

        std::map<int16_t, int32_t> v1_32;
        for (int32_t i; i < 66535; ++i) {
            v1_8.insert({ i, 1 });
        }

        Packer::pack(data, v1_8, v1_16, v1_32);

        std::map<int16_t, int32_t> v2_8;
        std::map<int16_t, int32_t> v2_16;
        std::map<int16_t, int32_t> v2_32;

        bool ok = UnPacker::unpack(data, v2_8, v2_16, v2_32);

        assert(ok);
        assert(v1_8 == v2_8);
        assert(v1_16 == v2_16);
        assert(v1_32 == v2_32);
    }

    // custom
    if (ALL_TEST || CUSTOM_TEST) {
        std::vector<uint8_t> data;

        ObjectWithPack obj1_1;
        obj1_1.i = 42;
        obj1_1.str = "42";

        ObjectWithPackUnPack obj1_2;
        obj1_2.i = 42;
        obj1_2.str = "42";

        ObjectCustomPackUnPack obj1_3;
        obj1_3.i = 42;
        obj1_3.str = "42";

        ObjectCustomLowLevel obj1_4;
        obj1_4.i = 42;
        obj1_4.str = "42";

        Packer::pack(data, obj1_1, obj1_2, obj1_3, obj1_4);

        ObjectWithPack obj2_1;
        ObjectWithPackUnPack obj2_2;
        ObjectCustomPackUnPack obj2_3;
        ObjectCustomLowLevel obj2_4;

        bool ok = UnPacker::unpack(data, obj2_1, obj2_2, obj2_3, obj2_4);

        assert(ok);
        assert(obj1_1 == obj2_1);
        assert(obj1_2 == obj2_2);
        assert(obj1_3 == obj2_3);
        assert(obj1_4 == obj2_4);
    }
}
