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
#include <optional>

#include "types/ret.h"

using namespace muse;

class Global_RetTests : public ::testing::Test
{
public:
    enum class StatusCode {
        OK, Error
    };
};

// === Valid Retrieval ===

TEST(Global_RetTests, Data_KeyExistsWithMatchingType_ReturnsValue)
{
    Ret ret;
    ret.setData("title", std::string("My Title"));
    const auto result = ret.data<std::string>("title");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "My Title");
}

TEST(Global_RetTests, Data_KeyExistsWithMatchingIntType_ReturnsValue)
{
    Ret ret;
    ret.setData("answer", 42);
    const auto result = ret.data<int>("answer");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 42);
}

// === Key Not Found ===

TEST(Global_RetTests, Data_KeyNotFound_ReturnsNulloptInReleaseANDTriggersAssertionInDebug)
{
#ifdef NDEBUG
    // In release builds, assertions are disabled, so tests normal behaviour
    const Ret ret;
    const auto result = ret.data<std::string>("nonexistent");
    EXPECT_FALSE(result.has_value());
#else
    // In debug builds, expect the assertion to trigger
    const Ret ret;
    EXPECT_DEATH({
        const auto result = ret.data<std::string>("nonexistent");
    }, ".*Assertion.*failed");
#endif
}

TEST(Global_RetTests, Data_KeyNotFoundWithDefault_ReturnsDefaultInReleaseANDTriggersAssertionInDebug)
{
#ifdef NDEBUG
    // In release builds, assertions are disabled, so tests normal behaviour
    const Ret ret;
    const auto result = ret.data<std::string>("missing", std::string("fallback"));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "fallback");
#else
    // In debug builds, expect the assertion to trigger even with default value
    const Ret ret;
    EXPECT_DEATH({
        const auto result = ret.data<std::string>("missing", std::string("fallback"));
    }, ".*Assertion.*failed");
#endif
}

// === Type Mismatch ===

TEST(Global_RetTests, Data_KeyExistsButTypeMismatch_ReturnsNulloptInReleaseANDTriggersAssertionInDebug)
{
    Ret ret;
    ret.setData("answer", 42); // int
#ifdef NDEBUG
    // In release builds, assertions are disabled, so tests normal behaviour
    const auto result = ret.data<std::string>("answer");
    EXPECT_FALSE(result.has_value());
#else
    // In debug builds, expect the assertion to trigger
    EXPECT_DEATH({
        const auto result = ret.data<std::string>("answer");
    }, ".*Assertion.*failed");
#endif
}

TEST(Global_RetTests, Data_KeyExistsButWrongCast_ReturnsNulloptInReleaseANDTriggersAssertionInDebug)
{
    Ret ret;
    ret.setData("version", std::string("1.0.0"));
#ifdef NDEBUG
    // In release builds, assertions are disabled, so tests normal behaviour
    const auto result = ret.data<int>("version");
    EXPECT_FALSE(result.has_value());
#else
    // In debug builds, expect the assertion to trigger
    EXPECT_DEATH({
        const auto result = ret.data<int>("version");
    }, ".*Assertion.*failed");
#endif
}

TEST(Global_RetTests, Data_TypeMismatchWithDefault_ReturnsDefaultInReleaseANDTriggersAssertionInDebug)
{
    Ret ret;
    ret.setData("active", true);
#ifdef NDEBUG
    // In release builds, assertions are disabled, so tests normal behaviour
    const auto result = ret.data<int>("active", 99);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 99);
#else
    // In debug builds, expect the assertion to trigger even with default value
    EXPECT_DEATH({
        const auto result = ret.data<int>("active", 99);
    }, ".*Assertion.*failed");
#endif
}

TEST(Global_RetTests, Data_TypeMismatchWithOptionalDefault_ReturnsOptionalDefaultInReleaseANDTriggersAssertionInDebug)
{
    Ret ret;
    ret.setData("username", std::string("admin"));
#ifdef NDEBUG
    // In release builds, assertions are disabled, so tests normal behaviour
    constexpr auto defaultOpt = std::optional<int> { 1234 };
    const auto result = ret.data<int>("username", defaultOpt);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), defaultOpt);
#else
    // In debug builds, expect the assertion to trigger even with default value
    EXPECT_DEATH({
        constexpr auto defaultOpt = std::optional<int> { 1234 };
        const auto result = ret.data<int>("username", defaultOpt);
    }, ".*Assertion.*failed");
#endif
}

// === Enum Class Support ===

TEST(Global_RetTests, Data_EnumClassStoredAndRetrievedCorrectly)
{
    Ret ret;
    ret.setData("status", Global_RetTests::StatusCode::OK);
    const auto result = ret.data<Global_RetTests::StatusCode>("status");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), Global_RetTests::StatusCode::OK);
}
