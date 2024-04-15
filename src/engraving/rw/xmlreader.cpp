/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "xmlreader.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;

namespace mu::engraving {
XmlReader::~XmlReader()
{
}

PointF XmlReader::readPoint()
{
    assert(tokenType() == XmlStreamReader::StartElement);
#ifndef NDEBUG
    if (!hasAttribute("x")) {
        LOGD("XmlReader::readPoint: x attribute missing: %s", name().ascii());
        unknown();
    }
    if (!hasAttribute("y")) {
        LOGD("XmlReader::readPoint: y attribute missing: %s", name().ascii());
        unknown();
    }
#endif
    double x = doubleAttribute("x", 0.0);
    double y = doubleAttribute("y", 0.0);
    readNext();
    return PointF(x, y);
}

//---------------------------------------------------------
//   readColor
//---------------------------------------------------------

Color XmlReader::readColor()
{
    assert(tokenType() == XmlStreamReader::StartElement);
    Color c;
    c.setRed(intAttribute("r"));
    c.setGreen(intAttribute("g"));
    c.setBlue(intAttribute("b"));
    c.setAlpha(intAttribute("a", 255));
    skipCurrentElement();
    return c;
}

//---------------------------------------------------------
//   readSize
//---------------------------------------------------------

SizeF XmlReader::readSize()
{
    assert(tokenType() == XmlStreamReader::StartElement);
    SizeF p;
    p.setWidth(doubleAttribute("w", 0.0));
    p.setHeight(doubleAttribute("h", 0.0));
    skipCurrentElement();
    return p;
}

ScaleF XmlReader::readScale()
{
    assert(tokenType() == XmlStreamReader::StartElement);
    ScaleF p;
    p.setWidth(doubleAttribute("w", 0.0));
    p.setHeight(doubleAttribute("h", 0.0));
    skipCurrentElement();
    return p;
}

//---------------------------------------------------------
//   readRect
//---------------------------------------------------------

RectF XmlReader::readRect()
{
    assert(tokenType() == XmlStreamReader::StartElement);
    RectF p;
    p.setLeft(doubleAttribute("x", 0.0));
    p.setTop(doubleAttribute("y", 0.0));
    p.setWidth(doubleAttribute("w", 0.0));
    p.setHeight(doubleAttribute("h", 0.0));
    skipCurrentElement();
    return p;
}

//---------------------------------------------------------
//   readFraction
//    recognizes this two styles:
//    <move z="2" n="4"/>     (old style)
//    <move>2/4</move>        (new style)
//---------------------------------------------------------

Fraction XmlReader::readFraction()
{
    assert(tokenType() == XmlStreamReader::StartElement);
    int z = intAttribute("z", 0);
    int n = intAttribute("n", 1);
    AsciiStringView s = readAsciiText();
    if (!s.empty()) {
        size_t i = s.indexOf('/');
        if (i == muse::nidx) {
            return Fraction::fromTicks(s.toInt());
        } else {
            String str = String::fromAscii(s.ascii());
            z = str.left(i).toInt();
            n = str.mid(i + 1).toInt();
        }
    }
    return Fraction(z, n);
}

//---------------------------------------------------------
//   unknown
//    unknown tag read
//---------------------------------------------------------

void XmlReader::unknown()
{
    if (XmlStreamReader::error()) {
        LOGD("%s ", muPrintable(errorString()));
    }
    if (!m_docName.isEmpty()) {
        LOGD() << "tag in <" << m_docName << "> line " << lineNumber() + m_offsetLines << " col "
               << columnNumber() << ": " << name();
    } else {
        LOGD() << "line " << lineNumber() + m_offsetLines << " col " << columnNumber() << ": " << name();
    }
    skipCurrentElement();
}

double XmlReader::readDouble(double min, double max)
{
    double val = readDouble();
    if (val < min) {
        val = min;
    } else if (val > max) {
        val = max;
    }
    return val;
}

//---------------------------------------------------------
//   htmlToString
//---------------------------------------------------------

void XmlReader::htmlToString(int level, String* s)
{
    *s += u'<' + String::fromAscii(name().ascii());
    for (const Attribute& a : attributes()) {
        *s += u' ' + String::fromAscii(a.name.ascii()) + u"=\"" + a.value + u'\"';
    }
    *s += u'>';
    ++level;
    for (;;) {
        XmlStreamReader::TokenType t = readNext();
        switch (t) {
        case XmlStreamReader::StartElement:
            htmlToString(level, s);
            break;
        case XmlStreamReader::EndElement:
            *s += u"</" + String::fromAscii(name().ascii()) + u'>';
            --level;
            return;
        case XmlStreamReader::Characters:
            if (!s->empty() || !isWhitespace()) {
                *s += text().toXmlEscaped();
            } else {
                LOGD() << "ignoring whitespace";
            }
            break;
        case XmlStreamReader::Comment:
            break;

        default:
            LOGD() << "htmlToString: read token: " << tokenString();
            return;
        }
    }
}

//-------------------------------------------------------------------
//   readXml
//    read verbatim until end tag of current level is reached
//-------------------------------------------------------------------

String XmlReader::readXml()
{
    String s;
    int level = 1;
    for (XmlStreamReader::TokenType t = readNext(); t != XmlStreamReader::EndElement; t = readNext()) {
        switch (t) {
        case XmlStreamReader::StartElement:
            htmlToString(level, &s);
            break;
        case XmlStreamReader::EndElement:
            break;
        case XmlStreamReader::Characters:
            if (!isWhitespace()) {
                s += text().toXmlEscaped();
            }
            break;
        case XmlStreamReader::Comment:
            break;

        default:
            LOGD() << "read token: " << tokenString();
            return s;
        }
    }
    return s;
}
}
