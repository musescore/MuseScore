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

namespace Ms {
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
    if (attributes().hasAttribute(s)) {
        // return attributes().value(s).toString().toInt();
        return attributes().value(s).toInt();
    } else {
        return _default;
    }
}

int XmlReader::intAttribute(const char* s) const
{
    return attributes().value(s).toInt();
}

//---------------------------------------------------------
//   doubleAttribute
//---------------------------------------------------------

double XmlReader::doubleAttribute(const char* s) const
{
    return attributes().value(s).toDouble();
}

double XmlReader::doubleAttribute(const char* s, double _default) const
{
    if (attributes().hasAttribute(s)) {
        return attributes().value(s).toDouble();
    } else {
        return _default;
    }
}

//---------------------------------------------------------
//   attribute
//---------------------------------------------------------

QString XmlReader::attribute(const char* s, const QString& _default) const
{
    if (attributes().hasAttribute(s)) {
        return attributes().value(s).toString();
    } else {
        return _default;
    }
}

//---------------------------------------------------------
//   hasAttribute
//---------------------------------------------------------

bool XmlReader::hasAttribute(const char* s) const
{
    return attributes().hasAttribute(s);
}

//---------------------------------------------------------
//   readPoint
//---------------------------------------------------------

PointF XmlReader::readPoint()
{
    Q_ASSERT(tokenType() == QXmlStreamReader::StartElement);
#ifndef NDEBUG
    if (!attributes().hasAttribute("x")) {
        QXmlStreamAttributes map = attributes();
        LOGD("XmlReader::readPoint: x attribute missing: %s (%d)",
             name().toUtf8().data(), map.size());
        for (int i = 0; i < map.size(); ++i) {
            const QXmlStreamAttribute& a = map.at(i);
            LOGD(" attr <%s> <%s>", a.name().toUtf8().data(), a.value().toUtf8().data());
        }
        unknown();
    }
    if (!attributes().hasAttribute("y")) {
        LOGD("XmlReader::readPoint: y attribute missing: %s", name().toUtf8().data());
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
    Q_ASSERT(tokenType() == QXmlStreamReader::StartElement);
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
    Q_ASSERT(tokenType() == QXmlStreamReader::StartElement);
    SizeF p;
    p.setWidth(doubleAttribute("w", 0.0));
    p.setHeight(doubleAttribute("h", 0.0));
    skipCurrentElement();
    return p;
}

ScaleF XmlReader::readScale()
{
    Q_ASSERT(tokenType() == QXmlStreamReader::StartElement);
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
    Q_ASSERT(tokenType() == QXmlStreamReader::StartElement);
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
    Q_ASSERT(tokenType() == QXmlStreamReader::StartElement);
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
    if (QXmlStreamReader::error()) {
        LOGD("%s ", qPrintable(errorString()));
    }
    if (!docName.isEmpty()) {
        LOGD("tag in <%s> line %lld col %lld: %s",
             qPrintable(docName), lineNumber() + _offsetLines, columnNumber(), name().toUtf8().data());
    } else {
        LOGD("line %lld col %lld: %s", lineNumber() + _offsetLines, columnNumber(), name().toUtf8().data());
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
    QXmlStreamReader::TokenType tt = readNext();
    if (tt == QXmlStreamReader::Characters) {
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
    *s += QString("<%1").arg(name().toString());
    for (const QXmlStreamAttribute& a : attributes()) {
        *s += QString(" %1=\"%2\"").arg(a.name().toString(), a.value().toString());
    }
    *s += ">";
    ++level;
    for (;;) {
        QXmlStreamReader::TokenType t = readNext();
        switch (t) {
        case QXmlStreamReader::StartElement:
            htmlToString(level, s);
            break;
        case QXmlStreamReader::EndElement:
            *s += QString("</%1>").arg(name().toString());
            --level;
            return;
        case QXmlStreamReader::Characters:
            if (!s->isEmpty() || !isWhitespace()) {
                *s += text().toString().toHtmlEscaped();
            } else {
                LOGD() << "ignoring whitespace";
            }
            break;
        case QXmlStreamReader::Comment:
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
    for (QXmlStreamReader::TokenType t = readNext(); t != QXmlStreamReader::EndElement; t = readNext()) {
        switch (t) {
        case QXmlStreamReader::StartElement:
            htmlToString(level, &s);
            break;
        case QXmlStreamReader::EndElement:
            break;
        case QXmlStreamReader::Characters:
            s += text().toString().toHtmlEscaped();
            break;
        case QXmlStreamReader::Comment:
            break;

        default:
            LOGD("htmlToString: read token: %s", qPrintable(tokenString()));
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

void XmlReader::setContext(mu::engraving::ReadContext* context)
{
    if (m_context && m_selfContext) {
        delete m_context;
    }

    m_context = context;
    m_selfContext = false;
}
}
