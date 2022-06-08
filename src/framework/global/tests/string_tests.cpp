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

TEST_F(Global_Types_StringTests, String_Compare)
{
    {
        //! GIVEN Some Strings
        String str1(u"123abc");
        String str2(u"123abc");
        String str3(u"abc123");
        String str4(u"abc");

        //! CHECK
        EXPECT_TRUE(str1 == str2);
        EXPECT_TRUE(str1 != str3);
        EXPECT_TRUE(str1 != str4);

        EXPECT_TRUE(str1 == u"123abc");
        EXPECT_TRUE(str1 != u"abc123");
        EXPECT_TRUE(str1 != u"abc");

        EXPECT_TRUE(u"123abc" == str1);
        EXPECT_TRUE(u"abc123" != str1);
        EXPECT_TRUE(u"abc" != str1);
    }

    {
        //! GIVEN Some ASCII Strings
        String str1(u"02");
        String str2(u"20");
        String str3(u"22");
        String str4(u"002");

        //! CHECK
        EXPECT_TRUE(str1 < str2);
        EXPECT_TRUE(str2 < str3);
        EXPECT_TRUE(str4 < str3);
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

TEST_F(Global_Types_StringTests, String_Replace)
{
    {
        //! GIVEN Some String
        String str = u"123abcПыф";
        //! DO
        str.replace(u"abc", u"def").replace(u"Пыф", u"Длж");
        //! CHECK
        EXPECT_EQ(str, u"123defДлж");
    }

    {
        //! GIVEN Some String
        String str = u"123abcПыф";
        //! DO
        str.replace(u"abc", u"de");
        //! CHECK
        EXPECT_EQ(str, u"123deПыф");
    }

    {
        //! GIVEN Some String
        String str = u"123abc456abc";
        //! DO
        str.replace(u"abc", u"de");
        //! CHECK
        EXPECT_EQ(str, u"123de456de");
    }
}

TEST_F(Global_Types_StringTests, String_Modify)
{
    {
        //! GIVEN Some String
        String str = u"123abc";
        //! DO
        String newStr = str + u"Пыф";
        //! CHECK
        EXPECT_EQ(newStr, u"123abcПыф");
    }

    {
        //! GIVEN Some String
        String str = u"123abc";
        //! DO
        str += u"Пыф";
        //! CHECK
        EXPECT_EQ(str, u"123abcПыф");
    }
}

TEST_F(Global_Types_StringTests, String_SubStr)
{
    {
        //! GIVEN Some String
        String str = u"123abc";
        //! DO
        String newStr = str.mid(2, 2);
        //! CHECK
        EXPECT_EQ(newStr, u"3a");
    }

    {
        //! GIVEN Some String
        String str = u"123abc";
        //! DO
        String newStr = str.mid(2);
        //! CHECK
        EXPECT_EQ(newStr, u"3abc");
    }
}

TEST_F(Global_Types_StringTests, String_Split)
{
    {
        //! GIVEN Some String
        String str = u"wwww gggg ыыыы";
        //! DO
        StringList list = str.split(u' ');
        //! CHECK
        EXPECT_EQ(list.size(), 3);
        EXPECT_EQ(list.at(0), u"wwww");
        EXPECT_EQ(list.at(1), u"gggg");
        EXPECT_EQ(list.at(2), u"ыыыы");
    }
}

TEST_F(Global_Types_StringTests, String_StartEndWith)
{
    {
        //! GIVEN Some String
        String str = u"123abc";
        //! CHECK
        EXPECT_TRUE(str.startsWith("12"));
        EXPECT_FALSE(str.startsWith("13"));
        //! CHECK
        EXPECT_TRUE(str.endsWith("bc"));
        EXPECT_FALSE(str.endsWith("ac"));
    }
}

TEST_F(Global_Types_StringTests, AsciiString_Construct)
{
    //! GIVEN Some ASCII String
    AsciiStringView str("123abc");
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
    AsciiStringView str("123abc");
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
        AsciiStringView str1("123abc");
        AsciiStringView str2("123abc");
        AsciiStringView str3("abc123");
        AsciiStringView str4("abc");

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
        AsciiStringView str1("02");
        AsciiStringView str2("20");
        AsciiStringView str3("22");
        AsciiStringView str4("002");

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
        AsciiStringView s("2");
        //! DO
        bool ok = false;
        int v = s.toInt(&ok);
        //! CHECK
        EXPECT_TRUE(ok);
        EXPECT_EQ(v, 2);
    }

    {
        //! GIVEN Some string
        AsciiStringView s("20");
        //! DO
        bool ok = false;
        int v = s.toInt(&ok);
        //! CHECK
        EXPECT_TRUE(ok);
        EXPECT_EQ(v, 20);
    }

    {
        //! GIVEN Some string
        AsciiStringView s("02");
        //! DO
        bool ok = false;
        int v = s.toInt(&ok);
        //! CHECK
        EXPECT_TRUE(ok);
        EXPECT_EQ(v, 2);
    }

    {
        //! GIVEN Some string
        AsciiStringView s(" 2");
        //! DO
        bool ok = false;
        int v = s.toInt(&ok);
        //! CHECK
        EXPECT_TRUE(ok);
        EXPECT_EQ(v, 2);
    }

    {
        //! GIVEN Some string
        AsciiStringView s("ab");
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
        AsciiStringView s("2");
        //! DO
        bool ok = false;
        double v = s.toDouble(&ok);
        //! CHECK
        EXPECT_TRUE(ok);
        EXPECT_DOUBLE_EQ(v, 2.0);
    }

    {
        //! GIVEN Some string
        AsciiStringView s("2.1");
        //! DO
        bool ok = false;
        double v = s.toDouble(&ok);
        //! CHECK
        EXPECT_TRUE(ok);
        EXPECT_DOUBLE_EQ(v, 2.1);
    }

    {
        //! GIVEN Some string
        AsciiStringView s("2,1");
        //! DO
        bool ok = false;
        double v = s.toDouble(&ok);
        //! CHECK Parsed just to `,`
        EXPECT_TRUE(ok);
        EXPECT_DOUBLE_EQ(v, 2);
    }

    {
        //! GIVEN Some string
        AsciiStringView s("ab");
        //! DO
        bool ok = false;
        double v = s.toDouble(&ok);
        //! CHECK
        EXPECT_FALSE(ok);
        EXPECT_DOUBLE_EQ(v, 0.0);
    }
}
