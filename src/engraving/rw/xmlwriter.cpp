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

#include "libmscore/property.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
//---------------------------------------------------------
//   Xml
//---------------------------------------------------------

XmlWriter::XmlWriter(Score* s)
{
    _score = s;
    setCodec("UTF-8");
}

XmlWriter::XmlWriter(Score* s, QIODevice* device)
    : QTextStream(device)
{
    _score = s;
    setCodec("UTF-8");
}

//---------------------------------------------------------
//   pTag
//---------------------------------------------------------

void XmlWriter::pTag(const char* name, PlaceText place)
{
    const char* tags[] = {
        "auto", "above", "below", "left"
    };
    tag(name, tags[int(place)]);
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

void XmlWriter::writeHeader()
{
    *this << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
}

//---------------------------------------------------------
//   stag
//    <mops attribute="value">
//---------------------------------------------------------

void XmlWriter::startObject(const QString& s)
{
    putLevel();
    *this << '<' << s << '>' << Qt::endl;
    stack.append(s.split(' ')[0]);
}

//---------------------------------------------------------
//   stag
//    <mops attribute="value">
//---------------------------------------------------------

void XmlWriter::startObject(const EngravingObject* se, const QString& attributes)
{
    startObject(se->name(), se, attributes);
}

//---------------------------------------------------------
//   stag
//    <mops attribute="value">
//---------------------------------------------------------

void XmlWriter::startObject(const QString& name, const EngravingObject* se, const QString& attributes)
{
    putLevel();
    *this << '<' << name;
    if (!attributes.isEmpty()) {
        *this << ' ' << attributes;
    }
    *this << '>' << Qt::endl;
    stack.append(name);

    if (_recordElements) {
        _elements.emplace_back(se, name);
    }
}

//---------------------------------------------------------
//   etag
//    </mops>
//---------------------------------------------------------

void XmlWriter::endObject()
{
    putLevel();
    *this << "</" << stack.takeLast() << '>' << Qt::endl;
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
    *this << "/>" << Qt::endl;
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
    *this << "</" << s << '>' << Qt::endl;
}

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void XmlWriter::tag(Pid id, const PropertyValue& data, const PropertyValue& def)
{
    if (data == def) {
        return;
    }
    const char* name = propertyName(id);
    if (name == 0) {
        return;
    }

    const QString writableVal(propertyToString(id, data, /* mscx */ true));
    if (writableVal.isEmpty()) {
        tagProperty(name, data);
    } else {
        tagProperty(name, PropertyValue(writableVal));
    }
}

void XmlWriter::tagProperty(const char* name, const mu::engraving::PropertyValue& data)
{
    QString ename(QString(name).split(' ')[0]);

    switch (data.type()) {
    case P_TYPE::UNDEFINED:
        UNREACHABLE;
        break;
    // base
    case P_TYPE::BOOL:
        putLevel();
        *this << "<" << name << ">";
        *this << int(data.value<bool>());
        *this << "</" << ename << ">\n";
        break;
    case P_TYPE::INT:
        putLevel();
        *this << "<" << name << ">";
        *this << data.value<int>();
        *this << "</" << ename << ">\n";
        break;
    case P_TYPE::REAL:
        putLevel();
        *this << "<" << name << ">";
        *this << data.value<qreal>();
        *this << "</" << ename << ">\n";
        break;
    case P_TYPE::STRING:
        putLevel();
        *this << "<" << name << ">";
        *this << xmlString(data.value<QString>());
        *this << "</" << ename << ">\n";
        break;
    // geometry
    case P_TYPE::POINT: {
        PointF p = data.value<PointF>();
        tag(name, p);
    }
    break;
    case P_TYPE::SIZE: {
        putLevel();
        SizeF s = data.value<SizeF>();
        *this << QString("<%1 w=\"%2\" h=\"%3\"/>\n").arg(name).arg(s.width()).arg(s.height());
    }
    break;
    case P_TYPE::PATH:
        UNREACHABLE; //! TODO
        break;
    case P_TYPE::SCALE:
        UNREACHABLE; //! TODO
        break;
    case P_TYPE::SPATIUM:
        putLevel();
        *this << "<" << name << ">";
        *this << data.value<Spatium>().val();
        *this << "</" << ename << ">\n";
        break;
    case P_TYPE::MILLIMETRE:
        putLevel();
        *this << "<" << name << ">";
        *this << qreal(data.value<Millimetre>());
        *this << "</" << ename << ">\n";
        break;
        break;

    // draw
    case P_TYPE::COLOR: {
        putLevel();
        Color color(data.value<draw::Color>());
        *this << QString("<%1 r=\"%2\" g=\"%3\" b=\"%4\" a=\"%5\"/>\n")
            .arg(name).arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
    }
    break;
    case P_TYPE::ORNAMENT_STYLE: {
        putLevel();
//        switch (data.value<OrnamentStyle>()) {
//        case OrnamentStyle::BAROQUE:
//            return "baroque";
//        case OrnamentStyle::DEFAULT:
//            return "default";
//        }
//        break;
    } break;
    case P_TYPE::ALIGN: {
        putLevel();
        Align a = data.value<Align>();
        const char* h;
        if (a & Align::HCENTER) {
            h = "center";
        } else if (a & Align::RIGHT) {
            h = "right";
        } else {
            h = "left";
        }
        const char* v;
        if (a & Align::BOTTOM) {
            v = "bottom";
        } else if (a & Align::VCENTER) {
            v = "center";
        } else if (a & Align::BASELINE) {
            v = "baseline";
        } else {
            v = "top";
        }
        *this << QString("<%1>%2,%3</%1>\n").arg(name).arg(h, v);
    }
    break;
    case P_TYPE::PLACEMENT_V: {
        putLevel();
        *this << "<" << name << ">";
        switch (data.value<PlacementV>()) {
        case PlacementV::ABOVE:
            *this << "above";
            break;
        case PlacementV::BELOW:
            *this << "below";
            break;
        }
        *this << "</" << ename << ">\n";
    }
    break;
    case P_TYPE::PLACEMENT_H: {
        putLevel();
        *this << "<" << name << ">";
        switch (data.value<PlacementH>()) {
        case PlacementH::LEFT:
            *this << "left";
            break;
        case PlacementH::CENTER:
            *this << "center";
            break;
        case PlacementH::RIGHT:
            *this << "right";
            break;
        }
        *this << "</" << ename << ">\n";
    }
    break;
    case P_TYPE::DIRECTION_V: {
        putLevel();
        switch (data.value<DirectionV>()) {
        case DirectionV::AUTO:
            *this << "auto";
            break;
        case DirectionV::UP:
            *this << "up";
            break;
        case DirectionV::DOWN:
            *this << "down";
            break;
        }
    }
    break;
    case P_TYPE::DIRECTION_H: {
        UNREACHABLE; //! TODO
    }
    break;
    // time
    case P_TYPE::FRACTION: {
        const Fraction& f = data.value<Fraction>();
        tag(name, f);
    }
    break;
    default: {
        UNREACHABLE; //! TODO
    }
    break;

//    case P_TYPE::TDURATION,
//    case P_TYPE::LAYOUT_BREAK,
//    case P_TYPE::VALUE_TYPE,
//    case P_TYPE::BEAM_MODE,

//    case P_TYPE::TEXT_PLACE,
//    case P_TYPE::TEMPO,
//    case P_TYPE::GROUPS,
//    case P_TYPE::SYMID,
//    case P_TYPE::INT_LIST,
//    case P_TYPE::GLISS_STYLE,
//    case P_TYPE::BARLINE_TYPE,
//    case P_TYPE::HEAD_TYPE,          // enum class Notehead::Type
//    case P_TYPE::HEAD_GROUP,         // enum class Notehead::Group
//    case P_TYPE::ZERO_INT,           // displayed with offset +1

//    case P_TYPE::SUB_STYLE,

//    case P_TYPE::CHANGE_METHOD,      // enum class VeloChangeMethod (for single note dynamics)
//    case P_TYPE::CHANGE_SPEED,       // enum class Dynamic::Speed
//    case P_TYPE::CLEF_TYPE,          // enum class ClefType
//    case P_TYPE::DYNAMIC_TYPE,       // enum class DynamicType
//    case P_TYPE::KEYMODE,            // enum class KeyMode
//    case P_TYPE::ORIENTATION,        // enum class Orientation

//    case P_TYPE::HEAD_SCHEME,        // enum class NoteHead::Scheme

//    case P_TYPE::PITCH_VALUES,
//    case P_TYPE::HOOK_TYPE
    }
}

void XmlWriter::tag(const char* name, const mu::PointF& p)
{
    putLevel();
    *this << QString("<%1 x=\"%2\" y=\"%3\"/>\n").arg(name).arg(p.x()).arg(p.y());
}

void XmlWriter::tag(const char* name, const Fraction& v, const Fraction& def)
{
    if (v == def) {
        return;
    }

    putLevel();
    *this << QString("<%1>%2/%3</%1>\n").arg(name).arg(v.numerator()).arg(v.denominator());
}

//---------------------------------------------------------
//   tag
//    <mops>value</mops>
//---------------------------------------------------------

void XmlWriter::tag(const char* name, QVariant data, QVariant defaultData)
{
    if (data != defaultData) {
        tag(QString(name), data);
    }
}

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
    case QVariant::LongLong:
        *this << "<" << name << ">";
        *this << data.toLongLong();
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
        const QRect& r(data.value<QRect>());
        *this << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg(name).arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height());
    }
    break;
    case QVariant::RectF:
    {
        const QRectF& r(data.value<QRectF>());
        *this << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg(name).arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height());
    }
    break;
    case QVariant::PointF:
    {
        const QPointF& p(data.value<QPointF>());
        *this << QString("<%1 x=\"%2\" y=\"%3\"/>\n").arg(name).arg(p.x()).arg(p.y());
    }
    break;
    case QVariant::SizeF:
    {
        const QSizeF& p(data.value<QSizeF>());
        *this << QString("<%1 w=\"%2\" h=\"%3\"/>\n").arg(name).arg(p.width()).arg(p.height());
    }
    break;
    default: {
        UNREACHABLE;
    }
    break;
    }
}

//---------------------------------------------------------
//   comment
//---------------------------------------------------------

void XmlWriter::comment(const QString& text)
{
    putLevel();
    *this << "<!-- " << text << " -->" << Qt::endl;
}

//---------------------------------------------------------
//   xmlString
//---------------------------------------------------------

QString XmlWriter::xmlString(ushort c)
{
    switch (c) {
    case '<':
        return QLatin1String("&lt;");
    case '>':
        return QLatin1String("&gt;");
    case '&':
        return QLatin1String("&amp;");
    case '\"':
        return QLatin1String("&quot;");
    default:
        // ignore invalid characters in xml 1.0
        if ((c < 0x20 && c != 0x09 && c != 0x0A && c != 0x0D)) {
            return QString();
        }
        return QString(QChar(c));
    }
}

//---------------------------------------------------------
//   xmlString
//---------------------------------------------------------

QString XmlWriter::xmlString(const QString& s)
{
    QString escaped;
    escaped.reserve(s.size());
    for (int i = 0; i < s.size(); ++i) {
        ushort c = s.at(i).unicode();
        escaped += xmlString(c);
    }
    return escaped;
}

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void XmlWriter::dump(int len, const unsigned char* p)
{
    putLevel();
    int col = 0;
    setFieldWidth(5);
    setNumberFlags(numberFlags() | QTextStream::ShowBase);
    setIntegerBase(16);
    for (int i = 0; i < len; ++i, ++col) {
        if (col >= 16) {
            setFieldWidth(0);
            *this << Qt::endl;
            col = 0;
            putLevel();
            setFieldWidth(5);
        }
        *this << (p[i] & 0xff);
    }
    if (col) {
        *this << Qt::endl << Qt::dec;
    }
    setFieldWidth(0);
    setIntegerBase(10);
}

//---------------------------------------------------------
//   writeXml
//    string s is already escaped (& -> "&amp;")
//---------------------------------------------------------

void XmlWriter::writeXml(const QString& name, QString s)
{
    QString ename(name.split(' ')[0]);
    putLevel();
    for (int i = 0; i < s.size(); ++i) {
        ushort c = s.at(i).unicode();
        if (c < 0x20 && c != 0x09 && c != 0x0A && c != 0x0D) {
            s[i] = '?';
        }
    }
    *this << "<" << name << ">";
    *this << s;
    *this << "</" << ename << ">\n";
}

//---------------------------------------------------------
//   assignLocalIndex
//---------------------------------------------------------

int XmlWriter::assignLocalIndex(const Location& mainElementLocation)
{
    return _linksIndexer.assignLocalIndex(mainElementLocation);
}

//---------------------------------------------------------
//   canWrite
//---------------------------------------------------------

bool XmlWriter::canWrite(const EngravingItem* e) const
{
    if (!_clipboardmode) {
        return true;
    }
    return _filter.canSelect(e);
}

//---------------------------------------------------------
//   canWriteVoice
//---------------------------------------------------------

bool XmlWriter::canWriteVoice(int track) const
{
    if (!_clipboardmode) {
        return true;
    }
    return _filter.canSelectVoice(track);
}
}
