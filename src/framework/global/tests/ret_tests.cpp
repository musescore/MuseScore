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
#include "types/ret.h"

using namespace muse;

class Global_RetTests : public ::testing::Test
{
public:
    enum class StatusCode {
        OK, Error, Unknown
    };
};

// === Valid Retrieval ===

TEST(Global_RetTests, Data_KeyExistsWithMatchingType_ReturnsStoredValue)
{
    Ret ret;
    ret.setData("title", std::string("My Title"));
    const auto result = ret.data<std::string>("title", std::string("Default title"));
    EXPECT_EQ(result, "My Title");
}

TEST(Global_RetTests, Data_KeyExistsWithMatchingIntType_ReturnsStoredValue)
{
    Ret ret;
    ret.setData("answer", 42);
    const auto result = ret.data<int>("answer", 0);
    EXPECT_EQ(result, 42);
}

// === Key Not Found ===

TEST(Global_RetTests, Data_KeyNotFound_ReturnsDefaultInReleaseANDTriggersAssertionInDebug)
{
#ifdef NDEBUG
    const Ret ret;
    const auto result = ret.data<std::string>("missing", std::string("fallback"));
    EXPECT_EQ(result, "fallback");
#else
    const Ret ret;
    EXPECT_DEATH({
        const auto result = ret.data<std::string>("missing", std::string("fallback"));
    }, ".*Assertion.*failed");
#endif
}

// === Type Mismatch ===

TEST(Global_RetTests, Data_KeyExistsButTypeMismatch_ReturnsDefaultInReleaseANDTriggersAssertionInDebug)
{
    Ret ret;
    ret.setData("answer", 42); // int stored, but requesting as string
#ifdef NDEBUG
    const auto result = ret.data<std::string>("answer", std::string("0"));
    EXPECT_EQ(result, std::string("0"));
#else
    // In debug builds, expect the assertion to trigger
    EXPECT_DEATH({
        const auto result = ret.data<std::string>("answer", std::string("0"));
    }, ".*Assertion.*failed");
#endif
}

// === Bad Any Cast Protection ===

TEST(Global_RetTests, Data_BadAnyCast_ReturnsDefaultInReleaseANDTriggersAssertionInDebug)
{
    Ret ret;
    // This test is harder to trigger since type checking happens first
    // but the method has protection against std::bad_any_cast
    ret.setData("value", std::string("text"));

#ifdef NDEBUG
    const auto result = ret.data<int>("value", 999);
    EXPECT_EQ(result, 999);
#else
    EXPECT_DEATH({
        const auto result = ret.data<int>("value", 999);
    }, ".*Assertion.*failed");
#endif
}

// === Enum Class Support ===

TEST(Global_RetTests, Data_EnumClass_ReturnsStoredValue)
{
    Ret ret;
    ret.setData("status", Global_RetTests::StatusCode::OK);
    const auto result = ret.data<Global_RetTests::StatusCode>("status", Global_RetTests::StatusCode::Unknown);
    EXPECT_EQ(result, Global_RetTests::StatusCode::OK);
}

TEST(Global_RetTests, Data_EnumClassKeyNotFound_ReturnsDefaultInReleaseANDTriggersAssertionInDebug)
{
#ifdef NDEBUG
    const Ret ret;
    const auto result = ret.data<Global_RetTests::StatusCode>("missing", Global_RetTests::StatusCode::Unknown);
    EXPECT_EQ(result, Global_RetTests::StatusCode::Unknown);
#else
    const Ret ret;
    EXPECT_DEATH({
        const auto result = ret.data<Global_RetTests::StatusCode>("missing", Global_RetTests::StatusCode::Unknown);
    }, ".*Assertion.*failed");
#endif
}
