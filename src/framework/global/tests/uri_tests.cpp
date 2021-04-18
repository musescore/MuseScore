//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include <gtest/gtest.h>

#include "uri.h"

using namespace mu;

class UriTests : public ::testing::Test
{
public:
};

TEST_F(UriTests, Uri_Parce_Valid)
{
    //! GIVEN Valid uri as string

    Uri uri("musescore://some/path");

    EXPECT_TRUE(uri.isValid());
    EXPECT_EQ(uri.scheme(), "musescore");
    EXPECT_EQ(uri.path(), "some/path");
    EXPECT_EQ(uri.toString(), "musescore://some/path");
}

TEST_F(UriTests, Uri_Parce_NotValid)
{
    //! GIVEN Not valid uri as string

    Uri uri("//some/path");

    EXPECT_FALSE(uri.isValid());
}

TEST_F(UriTests, Uri_Parce_QueryAsUri)
{
    //! GIVEN Valid uriquery as string

    Uri uri("musescore://some/path?param1=value1&param2=value2");

    EXPECT_TRUE(uri.isValid());
    EXPECT_EQ(uri.scheme(), "musescore");
    EXPECT_EQ(uri.path(), "some/path");
    EXPECT_EQ(uri.toString(), "musescore://some/path");
}

TEST_F(UriTests, UriQuery_Parce)
{
    //! GIVEN Valid uriquery as string

    UriQuery q("musescore://some/path?param1=value1&param2=value2");

    EXPECT_TRUE(q.isValid());
    EXPECT_EQ(q.uri().scheme(), "musescore");
    EXPECT_EQ(q.uri().path(), "some/path");
    EXPECT_EQ(q.uri().toString(), "musescore://some/path");

    EXPECT_EQ(q.params().size(), 2);
    EXPECT_EQ(q.param("param1"), Val("value1"));
    EXPECT_EQ(q.param("param2"), Val("value2"));
}

TEST_F(UriTests, UriQuery_Parce_Quoted)
{
    //! GIVEN Valid uriquery as string

    UriQuery q("musescore://some/path?param1=value1&param2='value2'&param3='x=5'");

    EXPECT_TRUE(q.isValid());
    EXPECT_EQ(q.uri().scheme(), "musescore");
    EXPECT_EQ(q.uri().path(), "some/path");
    EXPECT_EQ(q.uri().toString(), "musescore://some/path");

    EXPECT_EQ(q.params().size(), 3);
    EXPECT_EQ(q.param("param1"), Val("value1"));
    EXPECT_EQ(q.param("param2"), Val("value2"));
    EXPECT_EQ(q.param("param3"), Val("x=5"));
}
