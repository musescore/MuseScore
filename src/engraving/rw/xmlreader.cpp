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

#include "xml.h"

#include "libmscore/beam.h"
#include "libmscore/measure.h"
#include "libmscore/score.h"
#include "libmscore/spanner.h"
#include "libmscore/staff.h"
#include "libmscore/textlinebase.h"
#include "libmscore/tuplet.h"
#include "libmscore/linkedobjects.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   ~XmlReader
//---------------------------------------------------------

XmlReader::~XmlReader()
{
    if (m_selfContext) {
        delete m_context;
    }
}

//---------------------------------------------------------
//   intAttribute
//---------------------------------------------------------

int XmlReader::intAttribute(const char* s, int _default) const
{
    if (hasAttribute(s)) {
        return attribute(s).toInt();
    } else {
        return _default;
    }
}

int XmlReader::intAttribute(const char* s) const
{
    return attribute(s).toInt();
}

//---------------------------------------------------------
//   doubleAttribute
//---------------------------------------------------------

double XmlReader::doubleAttribute(const char* s) const
{
    return attribute(s).toDouble();
}

double XmlReader::doubleAttribute(const char* s, double _default) const
{
    if (hasAttribute(s)) {
        return attribute(s).toDouble();
    } else {
        return _default;
    }
}

//---------------------------------------------------------
//   attribute
//---------------------------------------------------------

QString XmlReader::attribute(const char* s, const QString& _default) const
{
    if (hasAttribute(s)) {
        return attribute(s);
    } else {
        return _default;
    }
}

//---------------------------------------------------------
//   hasAttribute
//---------------------------------------------------------

bool XmlReader::hasAttribute(const char* s) const
{
    return mu::XmlStreamReader::hasAttribute(s);
}

//---------------------------------------------------------
//   readPoint
//---------------------------------------------------------

PointF XmlReader::readPoint()
{
    Q_ASSERT(tokenType() == XmlStreamReader::StartElement);
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
    qreal x = doubleAttribute("x", 0.0);
    qreal y = doubleAttribute("y", 0.0);
    readNext();
    return PointF(x, y);
}

//---------------------------------------------------------
//   readColor
//---------------------------------------------------------

mu::draw::Color XmlReader::readColor()
{
    Q_ASSERT(tokenType() == XmlStreamReader::StartElement);
    mu::draw::Color c;
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
    Q_ASSERT(tokenType() == XmlStreamReader::StartElement);
    SizeF p;
    p.setWidth(doubleAttribute("w", 0.0));
    p.setHeight(doubleAttribute("h", 0.0));
    skipCurrentElement();
    return p;
}

ScaleF XmlReader::readScale()
{
    Q_ASSERT(tokenType() == XmlStreamReader::StartElement);
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
    Q_ASSERT(tokenType() == XmlStreamReader::StartElement);
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
    Q_ASSERT(tokenType() == XmlStreamReader::StartElement);
    int z = attribute("z", "0").toInt();
    int n = attribute("n", "1").toInt();
    const QString& s(readElementText());
    if (!s.isEmpty()) {
        int i = s.indexOf('/');
        if (i == -1) {
            return Fraction::fromTicks(s.toInt());
        } else {
            z = s.leftRef(i).toInt();
            n = s.midRef(i + 1).toInt();
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
        LOGD("%s ", qPrintable(errorString()));
    }
    if (!docName.isEmpty()) {
        LOGD("tag in <%s> line %ld col %lld: %s", qPrintable(docName), lineNumber() + _offsetLines, columnNumber(), name().ascii());
    } else {
        LOGD("line %lld col %ld: %s", lineNumber() + _offsetLines, columnNumber(), name().ascii());
    }
    skipCurrentElement();
}

//---------------------------------------------------------
//   readDouble
//---------------------------------------------------------

double XmlReader::readDouble(double min, double max)
{
    double val = readElementText().toDouble();
    if (val < min) {
        val = min;
    } else if (val > max) {
        val = max;
    }
    return val;
}

//---------------------------------------------------------
//   readBool
//---------------------------------------------------------

bool XmlReader::readBool()
{
    bool val;
    XmlStreamReader::TokenType tt = readNext();
    if (tt == XmlStreamReader::Characters) {
        val = text().toInt() != 0;
        readNext();
    } else {
        val = true;
    }
    return val;
}

//---------------------------------------------------------
//   htmlToString
//---------------------------------------------------------

void XmlReader::htmlToString(int level, QString* s)
{
    *s += QString("<%1").arg(name().toQLatin1String());
    for (const Attribute& a : attributes()) {
        *s += QString(" %1=\"%2\"").arg(a.name.ascii(), a.value.toQString());
    }
    *s += ">";
    ++level;
    for (;;) {
        XmlStreamReader::TokenType t = readNext();
        switch (t) {
        case XmlStreamReader::StartElement:
            htmlToString(level, s);
            break;
        case XmlStreamReader::EndElement:
            *s += QString("</%1>").arg(name().toQLatin1String());
            --level;
            return;
        case XmlStreamReader::Characters:
            if (!s->isEmpty() || !isWhitespace()) {
                *s += text().toHtmlEscaped();
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

QString XmlReader::readXml()
{
    QString s;
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
                s += text().toHtmlEscaped();
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

engraving::ReadContext* XmlReader::context() const
{
    if (!m_context) {
        m_context = new engraving::ReadContext(nullptr);
        m_selfContext = true;
    }
    return m_context;
}

void XmlReader::setContext(ReadContext* context)
{
    if (m_context && m_selfContext) {
        delete m_context;
    }

    m_context = context;
    m_selfContext = false;
}
}
