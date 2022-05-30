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

#include <QString>

#include "types/string.h"

using namespace mu;

class Global_Types_StringTests : public ::testing::Test
{
public:
};

TEST_F(Global_Types_StringTests, Construct)
{
    {
        //! GIVEN Empty string
        String str;
        //! CHECK
        EXPECT_EQ(str.size(), 0);
        EXPECT_TRUE(str.empty());
    }

    {
        //! GIVEN Some string
        String str = "123abc";
        //! DO Get size
        size_t size = str.size();
        //! CHECK
        EXPECT_EQ(size, 6);
    }

    {
        //! GIVEN Some string on cyrilic (not ASCII)
        String str = "привет!";
        //! DO Get size
        size_t size = str.size();
        //! CHECK
        EXPECT_EQ(size, 7);
    }

    {
        //! GIVEN Some QString
        QString qstr = "123abcПыф";
        //! DO
        String str = qstr;
        //! CHECK
        EXPECT_EQ(str.size(), qstr.size());

        //! DO
        String str2;
        str2 = qstr;
        //! CHECK
        EXPECT_EQ(str2.size(), qstr.size());
    }
}

TEST_F(Global_Types_StringTests, Convert)
{
    {
        //! GIVEN Some QString
        QString qstr_origin = "123abcПыф";
        //! DO
        String str = String::fromQString(qstr_origin);
        //! CHECK
        EXPECT_EQ(str.size(), qstr_origin.size()); // both utf16, so size is equal

        //! DO back
        QString qstr = str.toQString();
        //! CHECK
        EXPECT_EQ(qstr, qstr_origin);
    }

    {
        //! GIVEN Some std::string
        std::string sstr_origin = "123abcПыф";
        //! DO
        String str = String::fromStdString(sstr_origin);
        //! CHECK
        EXPECT_EQ(str.size(), 9);

        //! DO back
        std::string sstr = str.toStdString();
        //! CHECK
        EXPECT_EQ(sstr, sstr_origin);
    }
}

TEST_F(Global_Types_StringTests, Access)
{
    {
        //! GIVEN Some String
        String str = "123abcПыф";
        //! DO
        Char c0 = str.at(0);
        //! CHECK
        EXPECT_EQ(c0, Char(u'1'));
        EXPECT_EQ(c0, Char('1'));
        EXPECT_EQ(c0, '1');
        EXPECT_EQ(c0, u'1');

        //! DO
        Char c6 = str.at(6);
        //! CHECK
        EXPECT_EQ(c6, Char(u'П'));
        EXPECT_EQ(c6, u'П');
    }
}
