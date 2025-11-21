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

#include "serialization/xmlstreamreader.h"
#include "serialization/xmlstreamwriter.h"
#include "types/bytearray.h"
#include "io/buffer.h"

using namespace muse;
using namespace muse::io;

class Global_Ser_XML : public ::testing::Test
{
public:
};

TEST_F(Global_Ser_XML, WriteRead)
{
    ByteArray data;
    Buffer buf(&data);

    // Write
    {
        buf.open(IODevice::WriteOnly);
        XmlStreamWriter xml(&buf);
        xml.startDocument();
        xml.startElement("parent");
        xml.comment(u"comment");

        xml.element("hello", u"world");

        xml.element("tag", { { "attr", true }, { "attr2", 10 }, { "attr3", "string" } });

        xml.startElement("tag");
        xml.element("anotherTag");
        xml.endElement();

        xml.endElement();
        xml.flush();

        EXPECT_NE(data.size(), 0);
        buf.close();
    }

    // Read
    {
        buf.open(IODevice::ReadOnly);
        XmlStreamReader xml(&buf);

        EXPECT_TRUE(xml.readNextStartElement());
        EXPECT_EQ(xml.name(), "parent");
        EXPECT_TRUE(xml.readNextStartElement());
        EXPECT_EQ(xml.name(), "hello");
        EXPECT_EQ(xml.readText(), "world");

        EXPECT_TRUE(xml.readNextStartElement());
        EXPECT_EQ(xml.name(), "tag");
        EXPECT_EQ(xml.intAttribute("attr"), 1);
        EXPECT_EQ(xml.intAttribute("attr2"), 10);
        EXPECT_EQ(xml.attribute("attr3"), "string");
        xml.readNext();

        EXPECT_TRUE(xml.readNextStartElement());
        EXPECT_EQ(xml.name(), "tag");

        EXPECT_TRUE(xml.readNextStartElement());
        EXPECT_EQ(xml.name(), "anotherTag");
    }
}
