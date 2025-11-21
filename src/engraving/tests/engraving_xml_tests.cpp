/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "engraving/rw/xmlreader.h"
#include "engraving/rw/xmlwriter.h"
#include "thirdparty/kors_logger/src/log_base.h"
#include "types/bytearray.h"
#include "io/buffer.h"

using namespace mu;
using namespace mu::engraving;
using namespace muse;
using namespace muse::io;

class Engraving_XMLTests : public ::testing::Test
{
};

TEST_F(Engraving_XMLTests, readHTML)
{
    ByteArray data;
    Buffer buf(&data);
    static const String XML_TEXT_REF = u"<tag1><br/></tag1>";
    static const String XML_TEXT_VERBOSE = u"<tag1><br></br></tag1>";

    // Write
    {
        buf.open(IODevice::WriteOnly);
        XmlWriter xml(&buf);
        xml.startDocument();
        xml.startElement("parent");

        xml.writeXml(u"xmlTag", XML_TEXT_REF);
        xml.writeXml(u"xmlTag", XML_TEXT_VERBOSE);

        xml.endElement();
        xml.flush();

        EXPECT_NE(data.size(), 0);
        buf.close();

        LOGI() << buf.data().constChar();
    }

    // Read
    {
        buf.open(IODevice::ReadOnly);
        XmlReader xml(&buf);

        EXPECT_TRUE(xml.readNextStartElement());
        EXPECT_EQ(xml.name(), "parent");

        EXPECT_TRUE(xml.readNextStartElement());
        EXPECT_EQ(xml.name(), "xmlTag");
        String xmlTag = xml.readXml();
        LOGI() << xmlTag;

        // Expect that <br></br> is condensed to <br/>
        EXPECT_TRUE(xml.readNextStartElement());
        EXPECT_EQ(xml.name(), "xmlTag");
        String xmlTagVerbose = xml.readXml();
        LOGI() << xmlTagVerbose;
        EXPECT_EQ(xmlTag, XML_TEXT_REF);
    }
}
