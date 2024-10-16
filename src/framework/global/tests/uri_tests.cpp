/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "types/uri.h"
#include "types/val.h"

using namespace muse;

class Global_UriTests : public ::testing::Test
{
public:
};

TEST_F(Global_UriTests, Uri_Parse_Valid)
{
    //! GIVEN Valid uri as string

    Uri uri("muse://some/path");

    EXPECT_TRUE(uri.isValid());
    EXPECT_EQ(uri.scheme(), "muse");
    EXPECT_EQ(uri.path(), "some/path");
    EXPECT_EQ(uri.toString(), "muse://some/path");
}

TEST_F(Global_UriTests, Uri_Parse_NotValid)
{
    //! GIVEN Not valid uri as string

    Uri uri("//some/path");

    EXPECT_FALSE(uri.isValid());
}

TEST_F(Global_UriTests, Uri_Parse_QueryAsUri)
{
    //! GIVEN Valid uriquery as string

    Uri uri("muse://some/path?param1=value1&param2=value2");

    EXPECT_TRUE(uri.isValid());
    EXPECT_EQ(uri.scheme(), "muse");
    EXPECT_EQ(uri.path(), "some/path");
    EXPECT_EQ(uri.toString(), "muse://some/path");
}

TEST_F(Global_UriTests, UriQuery_Parse)
{
    //! GIVEN Valid uriquery as string

    UriQuery q("muse://some/path?param1=value1&param2=value2");

    EXPECT_TRUE(q.isValid());
    EXPECT_EQ(q.uri().scheme(), "muse");
    EXPECT_EQ(q.uri().path(), "some/path");
    EXPECT_EQ(q.uri().toString(), "muse://some/path");

    EXPECT_EQ(q.params().size(), 2);
    EXPECT_EQ(q.param("param1"), Val("value1"));
    EXPECT_EQ(q.param("param2"), Val("value2"));
}

TEST_F(Global_UriTests, UriQuery_Parse_Quoted)
{
    //! GIVEN Valid uriquery as string

    UriQuery q("muse://some/path?param1=value1&param2='value2'&param3='x=5'");

    EXPECT_TRUE(q.isValid());
    EXPECT_EQ(q.uri().scheme(), "muse");
    EXPECT_EQ(q.uri().path(), "some/path");
    EXPECT_EQ(q.uri().toString(), "muse://some/path");

    EXPECT_EQ(q.params().size(), 3);
    EXPECT_EQ(q.param("param1"), Val("value1"));
    EXPECT_EQ(q.param("param2"), Val("value2"));
    EXPECT_EQ(q.param("param3"), Val("x=5"));
}

TEST_F(Global_UriTests, UriQuery_ToString)
{
    //! GIVEN Valid uriquery

    UriQuery q("muse://some/path");
    q.addParam("param1", Val("value1"));

    //! DO to string
    std::string str = q.toString();
    EXPECT_EQ(str, "muse://some/path?param1=value1");

    //! DO add param 2
    q.addParam("param2", Val("value2"));

    //! Do to string
    std::string str2 = q.toString();
    EXPECT_EQ(str2, "muse://some/path?param1=value1&param2=value2");
}
