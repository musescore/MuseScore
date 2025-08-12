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
#include "serialization/xmldom.h"

using namespace muse;

namespace {
static ByteArray BA(const char* s) {
    return ByteArray(reinterpret_cast<const uint8_t*>(s), std::strlen(s));
}
static String S(const char* s) {
    return String::fromUtf8(s);
}
} // namespace

class Serialization_XmlDomTests : public ::testing::Test {};

// ---------- Parse + root element ----------
TEST_F(Serialization_XmlDomTests, ParseAndRootElement)
{
    const char* xml =
        "<?xml version=\"1.0\"?>\n"
        "<score>\n"
        "  <header key=\"C\" mode=\"maj\">Title</header>\n"
        "  <!-- comment -->\n"
        "  <body>\n"
        "    <part id=\"P1\"/>\n"
        "  </body>\n"
        "</score>\n";

    XmlDomDocument doc;
    doc.setContent(BA(xml));

    EXPECT_FALSE(doc.hasError()) << doc.errorString().toStdString();

    XmlDomElement root = doc.rootElement();
    EXPECT_FALSE(root.isNull());
    EXPECT_EQ(root.nodeName(), S("score"));
}

// ---------- Child traversal & filtering ----------
TEST_F(Serialization_XmlDomTests, ChildTraversalAndFiltering)
{
    const char* xml =
        "<a>"
        "  text1"
        "  <b/>"
        "  <!--c-->"
        "  <c><d/></c>"
        "</a>";

    XmlDomDocument doc;
    doc.setContent(BA(xml));
    XmlDomElement a = doc.rootElement();
    ASSERT_FALSE(a.isNull());
    EXPECT_EQ(a.nodeName(), S("a"));

    // firstChild(): could be text (whitespace/text), so just assert it’s not null
    XmlDomNode first = a.firstChild();
    EXPECT_FALSE(first.isNull());

    // firstChildElement(nullptr): first element child regardless of name
    XmlDomElement firstElem = a.firstChildElement(nullptr);
    ASSERT_FALSE(firstElem.isNull());
    EXPECT_EQ(firstElem.nodeName(), S("b"));

    // firstChildElement("c")
    XmlDomElement c = a.firstChildElement("c");
    ASSERT_FALSE(c.isNull());
    EXPECT_EQ(c.nodeName(), S("c"));

    // c.firstChildElement() == <d>
    XmlDomElement d = c.firstChildElement(nullptr);
    ASSERT_FALSE(d.isNull());
    EXPECT_EQ(d.nodeName(), S("d"));
}

// ---------- Sibling traversal (elements vs any) ----------
TEST_F(Serialization_XmlDomTests, SiblingTraversal)
{
    const char* xml =
        "<root>"
        "  <x/>text<!--com--><y/>"
        "</root>";

    XmlDomDocument doc;
    doc.setContent(BA(xml));
    XmlDomElement root = doc.rootElement();
    ASSERT_FALSE(root.isNull());

    XmlDomElement x = root.firstChildElement("x");
    ASSERT_FALSE(x.isNull());

    // nextSibling() may see text/comment; nextSiblingElement() should skip to <y>
    XmlDomNode sibAny = x.nextSibling();
    EXPECT_FALSE(sibAny.isNull()); // typically text

    XmlDomElement y = x.nextSiblingElement(nullptr);
    ASSERT_FALSE(y.isNull());
    EXPECT_EQ(y.nodeName(), S("y"));

    // previousSiblingElement from y should be x
    XmlDomElement prevElem = y.previousSiblingElement(nullptr);
    ASSERT_FALSE(prevElem.isNull());
    EXPECT_EQ(prevElem.nodeName(), S("x"));

    // parent()
    XmlDomNode p = y.parent();
    EXPECT_FALSE(p.isNull());
    EXPECT_EQ(p.toElement().nodeName(), S("root"));
}

// ---------- Attributes ----------
TEST_F(Serialization_XmlDomTests, AttributesAccess)
{
    const char* xml = "<e a=\"1\" b=\"two\" c=\"\"/>";
    XmlDomDocument doc;
    doc.setContent(BA(xml));
    XmlDomElement e = doc.rootElement();
    ASSERT_FALSE(e.isNull());

    // attribute(name)
    XmlDomAttribute a = e.attribute("a");
    ASSERT_FALSE(a.isNull());
    EXPECT_EQ(a.attributeName(), S("a"));
    EXPECT_EQ(a.value(), S("1"));

    XmlDomAttribute b = e.attribute("b");
    ASSERT_FALSE(b.isNull());
    EXPECT_EQ(b.value(), S("two"));

    XmlDomAttribute c = e.attribute("c");
    ASSERT_FALSE(c.isNull());
    EXPECT_EQ(c.value(), S(""));

    // firstAttribute / nextAttribute chain covers all three
    XmlDomAttribute it = e.firstAttribute();
    int count = 0;
    while (!it.isNull()) {
        ++count;
        it = it.nextAttribute();
    }
    EXPECT_EQ(count, 3);
}

// ---------- Element text aggregation ----------
TEST_F(Serialization_XmlDomTests, ElementText)
{
    const char* xml = "<e>hello <![CDATA[world]]><!--ignored--><x/>!</e>";
    XmlDomDocument doc;
    doc.setContent(BA(xml));
    XmlDomElement e = doc.rootElement();
    ASSERT_FALSE(e.isNull());

    // XmlDomElement::text() should concatenate PCDATA + CDATA only
    EXPECT_EQ(e.text(), S("hello world!"));
}

// ---------- Error handling (malformed XML) ----------
TEST_F(Serialization_XmlDomTests, ErrorHandlingMalformed)
{
    const char* bad = "<a><b></a>";
    XmlDomDocument doc;
    doc.setContent(BA(bad));

    EXPECT_TRUE(doc.hasError());
    EXPECT_FALSE(doc.errorString().empty());
}

// ---------- Empty document ----------
TEST_F(Serialization_XmlDomTests, EmptyDocument)
{
    const char* empty = "";
    XmlDomDocument doc;
    doc.setContent(BA(empty));
    EXPECT_TRUE(doc.hasError());
}
