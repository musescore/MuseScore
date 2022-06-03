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

#include "log.h"

using namespace mu;

class Global_Types_StringTests : public ::testing::Test
{
public:
};

TEST_F(Global_Types_StringTests, String_Construct)
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
        String str = u"123abc";
        //! DO Get size
        size_t size = str.size();
        //! CHECK
        EXPECT_EQ(size, 6);
    }

    {
        //! GIVEN Some string on cyrilic (not ASCII)
        String str = u"привет!";
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

TEST_F(Global_Types_StringTests, String_Convert)
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

TEST_F(Global_Types_StringTests, String_Access)
{
    {
        //! GIVEN Some String
        String str = u"123abcПыф";
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

TEST_F(Global_Types_StringTests, AsciiString_Construct)
{
    //! GIVEN Some ASCII String
    AsciiString str("123abc");
    //! DO
    size_t size = str.size();
    //! CHECK
    EXPECT_EQ(size, 6);

    //! DO
    const char* s = str.ascii();
    //! CHECK
    EXPECT_TRUE(std::strcmp(s, "123abc") == 0);
}

TEST_F(Global_Types_StringTests, AsciiString_Convert)
{
    //! GIVEN Some ASCII String
    AsciiString str("123abc");
    //! DO
    QLatin1String qstr = str.toQLatin1String();
    //! CHECK
    EXPECT_EQ(str.size(), qstr.size());
    EXPECT_TRUE(std::strcmp(str.ascii(), qstr.latin1()) == 0);
}

TEST_F(Global_Types_StringTests, AsciiString_Compare)
{
    {
        //! GIVEN Some ASCII Strings
        AsciiString str1("123abc");
        AsciiString str2("123abc");
        AsciiString str3("abc123");
        AsciiString str4("abc");

        //! CHECK
        EXPECT_TRUE(str1 == str2);
        EXPECT_TRUE(str1 != str3);
        EXPECT_TRUE(str1 != str4);

        EXPECT_TRUE(str1 == "123abc");
        EXPECT_TRUE(str1 != "abc123");
        EXPECT_TRUE(str1 != "abc");

        EXPECT_TRUE("123abc" == str1);
        EXPECT_TRUE("abc123" != str1);
        EXPECT_TRUE("abc" != str1);
    }

    {
        //! GIVEN Some ASCII Strings
        AsciiString str1("02");
        AsciiString str2("20");
        AsciiString str3("22");
        AsciiString str4("002");

        //! CHECK
        EXPECT_TRUE(str1 < str2);
        EXPECT_TRUE(str2 < str3);
        EXPECT_TRUE(str3 < str4);
    }
}

TEST_F(Global_Types_StringTests, AsciiString_ToInt)
{
    {
        //! GIVEN Some string
        AsciiString s("2");
        //! DO
        bool ok = false;
        int v = s.toInt(&ok);
        //! CHECK
        EXPECT_TRUE(ok);
        EXPECT_EQ(v, 2);
    }

    {
        //! GIVEN Some string
        AsciiString s("20");
        //! DO
        bool ok = false;
        int v = s.toInt(&ok);
        //! CHECK
        EXPECT_TRUE(ok);
        EXPECT_EQ(v, 20);
    }

    {
        //! GIVEN Some string
        AsciiString s("02");
        //! DO
        bool ok = false;
        int v = s.toInt(&ok);
        //! CHECK
        EXPECT_TRUE(ok);
        EXPECT_EQ(v, 2);
    }

    {
        //! GIVEN Some string
        AsciiString s(" 2");
        //! DO
        bool ok = false;
        int v = s.toInt(&ok);
        //! CHECK
        EXPECT_TRUE(ok);
        EXPECT_EQ(v, 2);
    }

    {
        //! GIVEN Some string
        AsciiString s("ab");
        //! DO
        bool ok = false;
        int v = s.toInt(&ok);
        //! CHECK
        EXPECT_FALSE(ok);
        EXPECT_EQ(v, 0);
    }
}

TEST_F(Global_Types_StringTests, AsciiString_ToDouble)
{
    {
        //! GIVEN Some string
        AsciiString s("2");
        //! DO
        bool ok = false;
        double v = s.toDouble(&ok);
        //! CHECK
        EXPECT_TRUE(ok);
        EXPECT_DOUBLE_EQ(v, 2.0);
    }

    {
        //! GIVEN Some string
        AsciiString s("2.1");
        //! DO
        bool ok = false;
        double v = s.toDouble(&ok);
        //! CHECK
        EXPECT_TRUE(ok);
        EXPECT_DOUBLE_EQ(v, 2.1);
    }

    {
        //! GIVEN Some string
        AsciiString s("2,1");
        //! DO
        bool ok = false;
        double v = s.toDouble(&ok);
        //! CHECK Parsed just to `,`
        EXPECT_TRUE(ok);
        EXPECT_DOUBLE_EQ(v, 2);
    }

    {
        //! GIVEN Some string
        AsciiString s("ab");
        //! DO
        bool ok = false;
        double v = s.toDouble(&ok);
        //! CHECK
        EXPECT_FALSE(ok);
        EXPECT_DOUBLE_EQ(v, 0.0);
    }
}
