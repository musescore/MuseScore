/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iterator>
#include <string>
#include <vector>

#include "global/stringutils.h"

using namespace std::literals;
using ::testing::ElementsAre;

namespace muse::strings {
TEST(Global_StringUtilsTests, testSplitEdgeCases)
{
    std::vector<std::string> parts;
    split(""s, std::back_inserter(parts), "");
    EXPECT_THAT(parts, ElementsAre(""s, ""s));

    parts.clear();
    split("foo"s, std::back_inserter(parts), "");
    EXPECT_THAT(parts, ElementsAre(""s, "f"s, "o"s, "o"s, ""s));

    parts.clear();
    split(""s, std::back_inserter(parts), "foo");
    EXPECT_THAT(parts, ElementsAre(""s));

    parts.clear();
    split("foo"s, std::back_inserter(parts), ",");
    EXPECT_THAT(parts, ElementsAre("foo"s));
}

TEST(Global_StringUtilsTests, testSplit)
{
    std::vector<std::string> parts;
    split("foo,bar,baz"s, std::back_inserter(parts), ",");
    EXPECT_THAT(parts, ElementsAre("foo"s, "bar"s, "baz"s));

    parts.clear();
    split(",foo,bar"s, std::back_inserter(parts), ",");
    EXPECT_THAT(parts, ElementsAre(""s, "foo"s, "bar"s));

    parts.clear();
    split("foo,bar,"s, std::back_inserter(parts), ",");
    EXPECT_THAT(parts, ElementsAre("foo"s, "bar"s, ""s));

    parts.clear();
    split("foo, bar,baz, quux"s, std::back_inserter(parts), ", ");
    EXPECT_THAT(parts, ElementsAre("foo"s, "bar,baz"s, "quux"s));
}
}
