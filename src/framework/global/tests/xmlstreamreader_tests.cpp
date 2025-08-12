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
#include <cstdint>
#include <cstring>

#include "types/bytearray.h"
#include "types/string.h"
#include "serialization/xmlstreamreader.h"

using namespace muse;

namespace {
static ByteArray BA(const char* s)
{
    return ByteArray(reinterpret_cast<const uint8_t*>(s), std::strlen(s));
}

static String S(const char* s)
{
    return String::fromUtf8(s);
}

// Helper: advance until a specific token or EndDocument/Invalid
static XmlStreamReader::TokenType advanceTo(XmlStreamReader& r, XmlStreamReader::TokenType want)
{
    for (;;) {
        XmlStreamReader::TokenType t = r.readNext();
        if (t == want || t == XmlStreamReader::TokenType::EndDocument || t == XmlStreamReader::TokenType::Invalid) {
            return t;
        }
    }
}
} // namespace

class Serialization_XmlStreamReaderTests : public ::testing::Test
{
};

// ---------- Happy path: declaration, root, attributes, text, comment, empty child ----------
TEST_F(Serialization_XmlStreamReaderTests, WalkAndReadBasics)
{
    const char* xml
        ="<?xml version=\"1.0\"?>\n"
         "<root a=\"1\" b=\"two\">hi<!--c--><child/>there</root>";

    XmlStreamReader xr;
    xr.setData(BA(xml));

    // First token should be StartDocument (XML declaration)
    EXPECT_EQ(xr.readNext(), XmlStreamReader::TokenType::StartDocument);

    // Next: StartElement(root)
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::StartElement), XmlStreamReader::TokenType::StartElement);
    EXPECT_TRUE(xr.isStartElement());
    EXPECT_EQ(xr.name(), AsciiStringView("root"));
    EXPECT_TRUE(xr.hasAttribute("a"));
    EXPECT_EQ(xr.asciiAttribute("a"), AsciiStringView("1"));
    EXPECT_EQ(xr.attribute("b"), S("two"));

    // Next Characters: "hi"
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::Characters), XmlStreamReader::TokenType::Characters);
    EXPECT_EQ(xr.asciiText(), AsciiStringView("hi"));

    // Skip comment
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::Comment), XmlStreamReader::TokenType::Comment);
    EXPECT_EQ(xr.asciiText(), AsciiStringView("c"));

    // child start/end
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::StartElement), XmlStreamReader::TokenType::StartElement);
    EXPECT_EQ(xr.name(), AsciiStringView("child"));
    // Empty element: we should see its EndElement next (or after any internal step)
    XmlStreamReader::TokenType t = advanceTo(xr, XmlStreamReader::TokenType::EndElement);
    EXPECT_EQ(t, XmlStreamReader::TokenType::EndElement);

    // Final Characters: "there"
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::Characters), XmlStreamReader::TokenType::Characters);
    EXPECT_EQ(xr.text(), S("there"));

    // Close root and then end document
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::EndElement), XmlStreamReader::TokenType::EndElement);
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::EndDocument), XmlStreamReader::TokenType::EndDocument);
}

// ---------- Expand more than one ENTITY value in DOCTYPE ----------
TEST_F(Serialization_XmlStreamReaderTests, EntityExpansion_MultipleEntitiesInDoctype)
{
    const char* xml
        ="<?xml version=\"1.0\"?>\n"
         "<!DOCTYPE r [ <!ENTITY HELLO \"Hello\"> <!ENTITY WHO \"World\"> ]>\n"
         "<r>&HELLO;, &WHO;!</r>";

    XmlStreamReader xr;
    xr.setData(BA(xml));

    EXPECT_EQ(xr.readNext(), XmlStreamReader::TokenType::StartDocument);
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::DTD), XmlStreamReader::TokenType::DTD);
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::StartElement), XmlStreamReader::TokenType::StartElement);
    EXPECT_EQ(xr.name(), AsciiStringView("r"));

    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::Characters), XmlStreamReader::TokenType::Characters);
    // EXPECTED (after fix): both entities expanded
    EXPECT_EQ(xr.text().toStdString(), "Hello, World!");
}

// ---------- Single-quoted entity values should be processed ----------
TEST_F(Serialization_XmlStreamReaderTests, EntityExpansion_SingleQuotedValue)
{
    const char* xml
        ="<?xml version=\"1.0\"?>\n"
         "<!DOCTYPE r [ <!ENTITY HELLO 'Hello'> ]>\n"
         "<r>&HELLO; world</r>";

    XmlStreamReader xr;
    xr.setData(BA(xml));

    EXPECT_EQ(xr.readNext(), XmlStreamReader::TokenType::StartDocument);
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::DTD), XmlStreamReader::TokenType::DTD);
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::StartElement), XmlStreamReader::TokenType::StartElement);
    EXPECT_EQ(xr.name(), AsciiStringView("r"));

    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::Characters), XmlStreamReader::TokenType::Characters);
    // EXPECTED (after fix): single-quoted value recognized
    EXPECT_EQ(xr.text().toStdString(), "Hello world");
}

// ---------- Name parsing should handle leading '%' and SYSTEM/PUBLIC noise ----------
TEST_F(Serialization_XmlStreamReaderTests, EntityExpansion_NameWithPercentAndKeywords)
{
    const char* xml
        ="<?xml version=\"1.0\"?>\n"
          // Leading spaces, optional '%', keywords, and extra spaces before the quoted value.
         "<!DOCTYPE r [ <!ENTITY    %   NAME   SYSTEM   \"X\"> <!ENTITY WHO \"World\"> ]>\n"
         "<r>&NAME;&WHO;</r>";

    XmlStreamReader xr;
    xr.setData(BA(xml));

    EXPECT_EQ(xr.readNext(), XmlStreamReader::TokenType::StartDocument);
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::DTD), XmlStreamReader::TokenType::DTD);
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::StartElement), XmlStreamReader::TokenType::StartElement);
    EXPECT_EQ(xr.name(), AsciiStringView("r"));

    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::Characters), XmlStreamReader::TokenType::Characters);
    // EXPECTED: name parsed as "NAME", value "X", and WHO expands too → "XWorld"
    EXPECT_EQ(xr.text().toStdString(), "XWorld");
}

// ---------- Name sanitizer should not strip "PUBLIC"/"SYSTEM" inside real names ----------
TEST_F(Serialization_XmlStreamReaderTests, EntityExpansion_NameSanitizerOverStrips)
{
    const char* xml
        ="<?xml version=\"1.0\"?>\n"
         "<!DOCTYPE r [ <!ENTITY PUBLICNAME \"X\"> <!ENTITY WHO \"World\"> ]>\n"
         "<r>&PUBLICNAME;&WHO;</r>";

    XmlStreamReader xr;
    xr.setData(BA(xml));

    // Need declaration and DTD visible (pugi: parse_declaration | parse_doctype flags)
    EXPECT_EQ(xr.readNext(), XmlStreamReader::TokenType::StartDocument);
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::DTD), XmlStreamReader::TokenType::DTD);
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::StartElement), XmlStreamReader::TokenType::StartElement);
    EXPECT_EQ(xr.name(), AsciiStringView("r"));

    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::Characters), XmlStreamReader::TokenType::Characters);

    // Correct behavior: both expand → "XWorld".
    EXPECT_EQ(xr.text().toStdString(), "XWorld");
}

// ---------- Error handling for malformed XML ----------
TEST_F(Serialization_XmlStreamReaderTests, MalformedXmlReportsError)
{
    const char* bad = "<a><b></a>";
    XmlStreamReader xr;
    xr.setData(BA(bad));

    // On malformed, readNext should quickly reach Invalid or EndDocument; error() should flag it
    XmlStreamReader::TokenType first = xr.readNext();
    // Depending on backend behavior, allow any of these early terminals
    bool terminal = (first == XmlStreamReader::TokenType::Invalid)
                    || (first == XmlStreamReader::TokenType::EndDocument)
                    || (first == XmlStreamReader::TokenType::NoToken);
    EXPECT_TRUE(terminal);

    EXPECT_NE(xr.error(), XmlStreamReader::Error::NoError);
    EXPECT_FALSE(xr.errorString().empty());
}

// ---------- Offset is non-zero on a visited node ----------
TEST_F(Serialization_XmlStreamReaderTests, OffsetProxyNonZeroOnNode)
{
    const char* xml = "<a>\n  <b>t</b>\n</a>\n";
    XmlStreamReader xr;
    xr.setData(BA(xml));

    // Step to StartElement <b>
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::StartElement), XmlStreamReader::TokenType::StartElement); // <a>
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::StartElement), XmlStreamReader::TokenType::StartElement); // <b>

    // With the pugi backend using offset as proxy, this should be > 0
    int64_t off = xr.byteOffset();
    EXPECT_GT(off, 0);
}

TEST_F(Serialization_XmlStreamReaderTests, ProcessingInstructionsIgnored)
{
    // PI appears inside the root element and should NOT produce any token.
    // We still expect the XML declaration as StartDocument.
    const char* xml
        ="<?xml version=\"1.0\"?>\n"
         "<root>hi<?proc do-this='now'?>"
         "<child/>there</root>";

    XmlStreamReader xr;
    xr.setData(BA(xml));

    // XML declaration → StartDocument
    EXPECT_EQ(xr.readNext(), XmlStreamReader::TokenType::StartDocument);

    // Root element
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::StartElement), XmlStreamReader::TokenType::StartElement);
    EXPECT_EQ(xr.name(), AsciiStringView("root"));

    // "hi"
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::Characters), XmlStreamReader::TokenType::Characters);
    EXPECT_EQ(xr.asciiText(), AsciiStringView("hi"));

    // Next token should jump directly to <child/> start — PI must be ignored
    XmlStreamReader::TokenType t = xr.readNext();
    ASSERT_EQ(t, XmlStreamReader::TokenType::StartElement);
    EXPECT_EQ(xr.name(), AsciiStringView("child"));

    // Empty element closes
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::EndElement), XmlStreamReader::TokenType::EndElement);

    // Then trailing "there"
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::Characters), XmlStreamReader::TokenType::Characters);
    EXPECT_EQ(xr.text(), S("there"));

    // Close out
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::EndElement), XmlStreamReader::TokenType::EndElement);
    EXPECT_EQ(advanceTo(xr, XmlStreamReader::TokenType::EndDocument), XmlStreamReader::TokenType::EndDocument);
}
