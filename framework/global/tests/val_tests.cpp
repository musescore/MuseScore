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

#include "val.h"

using namespace mu;

class ValTests : public ::testing::Test
{
public:
};

//Undefined
//Bool,
//Int,
//Double,
//String,
//Color
//Variant

TEST_F(ValTests, Val_Undefined)
{
    //! GIVEN Undefined value

    Val v;

    EXPECT_EQ(v.type(), Val::Type::Undefined);
    EXPECT_EQ(v.isNull(), true);
    EXPECT_EQ(v.toBool(), false);
    EXPECT_EQ(v.toInt(), 0);
    EXPECT_EQ(v.toDouble(), 0.0);
    EXPECT_EQ(v.toString(), std::string());
    EXPECT_EQ(v.toQColor(), QColor());
}

TEST_F(ValTests, Val_Bool)
{
    //! GIVEN Value as bool

    Val v(true);

    EXPECT_EQ(v.type(), Val::Type::Bool);
    EXPECT_EQ(v.isNull(), false);
    EXPECT_EQ(v.toBool(), true);
    EXPECT_EQ(v.toInt(), 1);
    EXPECT_EQ(v.toDouble(), 1.0);
    EXPECT_EQ(v.toString(), "true");
    EXPECT_EQ(v.toQColor(), QColor());
}

TEST_F(ValTests, Val_Int)
{
    //! GIVEN Value as int

    Val v(42);

    EXPECT_EQ(v.type(), Val::Type::Int);
    EXPECT_EQ(v.isNull(), false);
    EXPECT_EQ(v.toBool(), true);
    EXPECT_EQ(v.toInt(), 42);
    EXPECT_EQ(v.toDouble(), 42.0);
    EXPECT_EQ(v.toString(), "42");
    EXPECT_EQ(v.toQColor(), QColor());
}

TEST_F(ValTests, Val_Double)
{
    //! GIVEN Value as double

    Val v(42.42);

    EXPECT_EQ(v.type(), Val::Type::Double);
    EXPECT_EQ(v.isNull(), false);
    EXPECT_EQ(v.toBool(), true);
    EXPECT_EQ(v.toInt(), 42);
    EXPECT_EQ(v.toDouble(), 42.42);
    EXPECT_EQ(v.toString(), "42.42");
    EXPECT_EQ(v.toQColor(), QColor());
}

TEST_F(ValTests, Val_String)
{
    //! GIVEN Value as string

    Val v("hello");

    EXPECT_EQ(v.type(), Val::Type::String);
    EXPECT_EQ(v.isNull(), false);
    EXPECT_EQ(v.toBool(), true);
    EXPECT_EQ(v.toString(), "hello");
    EXPECT_EQ(v.toQColor(), QColor());
}

TEST_F(ValTests, Val_Color)
{
    //! GIVEN Value as color

    Val v(QColor("#800000"));

    EXPECT_EQ(v.type(), Val::Type::Color);
    EXPECT_EQ(v.isNull(), false);
    EXPECT_EQ(v.toBool(), true);
    EXPECT_EQ(v.toString(), "#800000");
    EXPECT_EQ(v.toQColor(), QColor("#800000"));
}

TEST_F(ValTests, Val_Variant)
{
    //! GIVEN Value as variant
    QVariantMap obj;
    obj["AAA"] = "some value";
    obj["BBB"] = "another value";
    obj["CCC"] = "test value";

    Val v(obj);

    EXPECT_EQ(v.type(), Val::Type::Variant);
    EXPECT_EQ(v.isNull(), false);
    EXPECT_EQ(v.toBool(), true);
    EXPECT_EQ(v.toQVariant(), obj);
}

TEST_F(ValTests, Val_Variant_Number)
{
    //! Given Value as variant with number type
    QVariant number(100500.539);
    Val v(number);

    EXPECT_EQ(v.type(), Val::Type::Variant);
    EXPECT_EQ(v.isNull(), false);
    EXPECT_EQ(v.toBool(), true);
    EXPECT_EQ(v.toInt(), 100501);
    EXPECT_EQ(v.toDouble(), 100500.539);
    EXPECT_EQ(v.toString(), "100500.539");
    EXPECT_EQ(v.toQVariant(), number);
}
