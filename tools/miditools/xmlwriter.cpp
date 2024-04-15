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

#include "xmlwriter.h"

#include <QSizeF>
#include <QRectF>
#include <QColor>

//---------------------------------------------------------
//   XmlWriter
//---------------------------------------------------------

XmlWriter::XmlWriter()
{
    stack.clear();
}

XmlWriter::XmlWriter(QIODevice* device)
    : QTextStream(device)
{
    setCodec("UTF-8");
    stack.clear();
}

//---------------------------------------------------------
//   putLevel
//---------------------------------------------------------

void XmlWriter::putLevel()
{
    int level = stack.size();
    for (int i = 0; i < level * 2; ++i) {
        *this << ' ';
    }
}

//---------------------------------------------------------
//   header
//---------------------------------------------------------

void XmlWriter::header()
{
    *this << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
}

//---------------------------------------------------------
//   stag
//    <mops attribute="value">
//---------------------------------------------------------

void XmlWriter::stag(const QString& s)
{
    putLevel();
    *this << '<' << s << '>' << endl;
    stack.append(s.split(' ')[0]);
}

//---------------------------------------------------------
//   etag
//    </mops>
//---------------------------------------------------------

void XmlWriter::etag()
{
    putLevel();
    *this << "</" << stack.takeLast() << '>' << endl;
}

//---------------------------------------------------------
//   tagE
//    <mops attribute="value"/>
//---------------------------------------------------------

void XmlWriter::tagE(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    putLevel();
    *this << '<';
    char buffer[BS];
    vsnprintf(buffer, BS, format, args);
    *this << buffer;
    va_end(args);
    *this << "/>" << endl;
}

//---------------------------------------------------------
//   tagE
//---------------------------------------------------------

void XmlWriter::tagE(const QString& s)
{
    putLevel();
    *this << '<' << s << "/>\n";
}

//---------------------------------------------------------
//   ntag
//    <mops> without newline
//---------------------------------------------------------

void XmlWriter::ntag(const char* name)
{
    putLevel();
    *this << "<" << name << ">";
}

//---------------------------------------------------------
//   netag
//    </mops>     without indentation
//---------------------------------------------------------

void XmlWriter::netag(const char* s)
{
    *this << "</" << s << '>' << endl;
}

//---------------------------------------------------------
//   tag
//    <mops>value</mops>
//---------------------------------------------------------

void XmlWriter::tag(const QString& name, QVariant data)
{
    QString ename(name.split(' ')[0]);

    putLevel();
    switch (data.type()) {
    case QVariant::Bool:
    case QVariant::Char:
    case QVariant::Int:
    case QVariant::UInt:
        *this << "<" << name << ">";
        *this << data.toInt();
        *this << "</" << ename << ">\n";
        break;
    case QVariant::Double:
        *this << "<" << name << ">";
        *this << data.value<double>();
        *this << "</" << ename << ">\n";
        break;
    case QVariant::String:
        *this << "<" << name << ">";
        *this << xmlString(data.value<QString>());
        *this << "</" << ename << ">\n";
        break;
    case QVariant::Color:
    {
        QColor color(data.value<QColor>());
        *this << QString("<%1 r=\"%2\" g=\"%3\" b=\"%4\" a=\"%5\"/>\n")
            .arg(name).arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
    }
    break;
    case QVariant::Rect:
    {
        QRect r(data.value<QRect>());
        *this
            << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg(name).arg(r.x()).arg(r.y()).arg(r.width()).arg(
            r.height());
    }
    break;
    case QVariant::RectF:
    {
        QRectF r(data.value<QRectF>());
        *this
            << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg(name).arg(r.x()).arg(r.y()).arg(r.width()).arg(
            r.height());
    }
    break;
    case QVariant::PointF:
    {
        QPointF p(data.value<QPointF>());
        *this << QString("<%1 x=\"%2\" y=\"%3\"/>\n").arg(name).arg(p.x()).arg(p.y());
    }
    break;
    case QVariant::SizeF:
    {
        QSizeF p(data.value<QSizeF>());
        *this << QString("<%1 w=\"%2\" h=\"%3\"/>\n").arg(name).arg(p.width()).arg(p.height());
    }
    break;
    default:
        qDebug("XmlWriter::tag: unsupported type %d", data.type());
        // abort();
        break;
    }
}

//---------------------------------------------------------
//   toHtml
//---------------------------------------------------------

QString XmlWriter::xmlString(const QString& s)
{
    QString escaped;
    escaped.reserve(s.size());
    for (int i = 0; i < s.size(); ++i) {
        QChar c = s.at(i);
        switch (c.unicode()) {
        case '<':
            escaped.append(QLatin1String("&lt;"));
            break;
        case '>':
            escaped.append(QLatin1String("&gt;"));
            break;
        case '&':
            escaped.append(QLatin1String("&amp;"));
            break;
        case '\"':
            escaped.append(QLatin1String("&quot;"));
            break;
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x0B:
        case 0x0C:
        case 0x0E:
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x17:
        case 0x18:
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
            break;
        default:
            escaped += QChar(c);
            break;
        }
    }
    return escaped;
}
