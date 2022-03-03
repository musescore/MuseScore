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

#include "xmlvalue.h"
#include "types/typesconv.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::rw;

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
    startObject(se->typeName(), se, attributes);
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

void XmlWriter::tag(Pid id, const PropertyValue& val, const PropertyValue& def)
{
    if (val == def) {
        return;
    }
    const char* name = propertyName(id);
    if (name == 0) {
        return;
    }

    P_TYPE valType = val.type();
    P_TYPE propType = propertyType(id);
    if (valType != propType) {
        LOGD() << "property value type mismatch, prop: " << name;
    }

    const QString writableVal(propertyToString(id, val, /* mscx */ true));
    if (writableVal.isEmpty()) {
        //! NOTE The data type is MILLIMETRE, but we write SPATIUM
        //! (the conversion from Millimetre to Spatium occurred higher up the stack)
        if (propType == P_TYPE::MILLIMETRE) {
            propType = P_TYPE::SPATIUM;
        }

        //! HACK Temporary hack. We have some kind of property with property type BOOL,
        //! but the used value type is INT (not just 1 and 0)
        //! see STAFF_BARLINE_SPAN
        if (propType == P_TYPE::BOOL && valType == P_TYPE::INT) {
            propType = P_TYPE::INT;
        }

        tagProperty(name, propType, val);
    } else {
        tagProperty(name, P_TYPE::STRING, PropertyValue(writableVal));
    }
}

void XmlWriter::tagProperty(const char* name, P_TYPE type, const PropertyValue& data)
{
    QString ename(QString(name).split(' ')[0]);

    switch (type) {
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
    case P_TYPE::DRAW_PATH:
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
    case P_TYPE::SYMID: {
        putLevel();
        *this << "<" << name << ">";
        *this << TConv::toXml(data.value<SymId>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::COLOR: {
        putLevel();
        Color color(data.value<Color>());
        *this << QString("<%1 r=\"%2\" g=\"%3\" b=\"%4\" a=\"%5\"/>\n")
            .arg(name).arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
    }
    break;
    case P_TYPE::ORNAMENT_STYLE: {
        putLevel();
        *this << "<" << name << ">";
        *this << XmlValue::toXml(data.value<OrnamentStyle>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::GLISS_STYLE: {
        putLevel();
        *this << "<" << name << ">";
        *this << XmlValue::toXml(data.value<GlissandoStyle>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::ALIGN: {
        putLevel();
        *this << "<" << name << ">";
        *this << TConv::toXml(data.value<Align>());
        *this << "</" << ename << ">\n";
    }
    break;
    case P_TYPE::PLACEMENT_V: {
        putLevel();
        *this << "<" << name << ">";
        *this << XmlValue::toXml(data.value<PlacementV>());
        *this << "</" << ename << ">\n";
    }
    break;
    case P_TYPE::PLACEMENT_H: {
        putLevel();
        *this << "<" << name << ">";
        *this << XmlValue::toXml(data.value<PlacementH>());
        *this << "</" << ename << ">\n";
    }
    break;
    case P_TYPE::TEXT_PLACE: {
        putLevel();
        *this << "<" << name << ">";
        *this << XmlValue::toXml(data.value<TextPlace>());
        *this << "</" << ename << ">\n";
    }
    break;
    case P_TYPE::DIRECTION_V: {
        putLevel();
        *this << "<" << name << ">";
        *this << XmlValue::toXml(data.value<DirectionV>());
        *this << "</" << ename << ">\n";
    }
    break;
    case P_TYPE::DIRECTION_H: {
        putLevel();
        *this << "<" << name << ">";
        *this << XmlValue::toXml(data.value<DirectionH>());
        *this << "</" << ename << ">\n";
    }
    break;
    case P_TYPE::ORIENTATION: {
        putLevel();
        *this << "<" << name << ">";
        *this << TConv::toXml(data.value<Orientation>());
        *this << "</" << ename << ">\n";
    }
    break;
    // time
    case P_TYPE::FRACTION: {
        const Fraction& f = data.value<Fraction>();
        tag(name, f);
    }
    break;
    case P_TYPE::LAYOUTBREAK_TYPE: {
        putLevel();
        *this << "<" << name << ">";
        *this << XmlValue::toXml(data.value<LayoutBreakType>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::VELO_TYPE: {
        putLevel();
        *this << "<" << name << ">";
        *this << XmlValue::toXml(data.value<VeloType>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::BARLINE_TYPE: {
        putLevel();
        *this << "<" << name << ">";
        *this << XmlValue::toXml(data.value<BarLineType>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::NOTEHEAD_TYPE: {
        putLevel();
        *this << "<" << name << ">";
        *this << TConv::toXml(data.value<NoteHeadType>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::NOTEHEAD_SCHEME: {
        putLevel();
        *this << "<" << name << ">";
        *this << TConv::toXml(data.value<NoteHeadScheme>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::NOTEHEAD_GROUP: {
        putLevel();
        *this << "<" << name << ">";
        *this << TConv::toXml(data.value<NoteHeadGroup>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::CLEF_TYPE: {
        putLevel();
        *this << "<" << name << ">";
        *this << TConv::toXml(data.value<ClefType>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::DYNAMIC_TYPE: {
        putLevel();
        *this << "<" << name << ">";
        *this << TConv::toXml(data.value<DynamicType>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::DYNAMIC_RANGE: {
        putLevel();
        *this << "<" << name << ">";
        *this << TConv::toXml(data.value<DynamicRange>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::DYNAMIC_SPEED: {
        putLevel();
        *this << "<" << name << ">";
        *this << TConv::toXml(data.value<DynamicSpeed>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::HOOK_TYPE: {
        putLevel();
        *this << "<" << name << ">";
        *this << TConv::toXml(data.value<HookType>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::KEY_MODE: {
        putLevel();
        *this << "<" << name << ">";
        *this << TConv::toXml(data.value<KeyMode>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::TEXT_STYLE: {
        putLevel();
        *this << "<" << name << ">";
        *this << TConv::toXml(data.value<TextStyleType>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::CHANGE_METHOD: {
        putLevel();
        *this << "<" << name << ">";
        *this << TConv::toXml(data.value<ChangeMethod>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::ACCIDENTAL_ROLE: {
        putLevel();
        *this << "<" << name << ">";
        *this << TConv::toXml(data.value<AccidentalRole>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::PLAYTECH_TYPE: {
        putLevel();
        *this << "<" << name << ">";
        *this << TConv::toXml(data.value<PlayingTechniqueType>());
        *this << "</" << ename << ">\n";
    } break;
    case P_TYPE::TEMPOCHANGE_TYPE: {
        putLevel();
        *this << "<" << name << ">";
        *this << TConv::toXml(data.value<TempoChangeType>());
        *this << "</" << ename << ">\n";
    } break;
    default: {
        UNREACHABLE; //! TODO
    }
    break;
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

mu::engraving::WriteContext* XmlWriter::context() const
{
    return m_context;
}

void XmlWriter::setContext(WriteContext* context)
{
    m_context = context;
}
}
