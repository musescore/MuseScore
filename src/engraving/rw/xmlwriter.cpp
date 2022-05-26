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

#include "types/typesconv.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
//---------------------------------------------------------
//   Xml
//---------------------------------------------------------

XmlWriter::XmlWriter()
{
}

XmlWriter::XmlWriter(QIODevice* device)
    : XmlStreamWriter(device)
{
}

XmlWriter::XmlWriter(mu::io::IODevice* device)
    : XmlStreamWriter(device)
{
}

XmlWriter::~XmlWriter()
{
    if (m_selfContext) {
        delete m_context;
    }
}

//---------------------------------------------------------
//   stag
//    <mops attribute="value">
//---------------------------------------------------------

void XmlWriter::startObject(const QString& s)
{
    XmlStreamWriter::writeStartElement(s);
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
    XmlStreamWriter::writeStartElement(name, attributes);

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
    XmlStreamWriter::writeEndElement();
}

//---------------------------------------------------------
//   tagE
//---------------------------------------------------------

void XmlWriter::tagE(const QString& s)
{
    XmlStreamWriter::writeElement(s);
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

void XmlWriter::tagProperty(const QString& name, const PropertyValue& val, const PropertyValue& def)
{
    if (val == def) {
        return;
    }

    tagProperty(name, val.type(), val);
}

void XmlWriter::tagProperty(const QString& name, P_TYPE type, const PropertyValue& data)
{
    switch (type) {
    case P_TYPE::UNDEFINED:
        UNREACHABLE;
        break;
    // base
    case P_TYPE::BOOL:
        XmlStreamWriter::writeElement(name, int(data.value<bool>()));
        break;
    case P_TYPE::INT:
        XmlStreamWriter::writeElement(name, data.value<int>());
        break;
    case P_TYPE::REAL:
        XmlStreamWriter::writeElement(name, data.value<qreal>());
        break;
    case P_TYPE::STRING:
        XmlStreamWriter::writeElement(name, xmlString(data.value<QString>()));
        break;
    // geometry
    case P_TYPE::POINT: {
        PointF p = data.value<PointF>();
        tag(name, p);
    }
    break;
    case P_TYPE::SIZE: {
        SizeF s = data.value<SizeF>();
        XmlStreamWriter::writeElement(QString("%1 w=\"%2\" h=\"%3\"").arg(name).arg(s.width()).arg(s.height()));
    }
    break;
    case P_TYPE::DRAW_PATH:
        UNREACHABLE; //! TODO
        break;
    case P_TYPE::SCALE: {
        ScaleF s = data.value<ScaleF>();
        XmlStreamWriter::writeElement(QString("%1 w=\"%2\" h=\"%3\"").arg(name).arg(s.width()).arg(s.height()));
    } break;
    case P_TYPE::SPATIUM:
        XmlStreamWriter::writeElement(name, data.value<Spatium>().val());
        break;
    case P_TYPE::MILLIMETRE:
        XmlStreamWriter::writeElement(name, data.value<Millimetre>().val());
        break;

    // draw
    case P_TYPE::SYMID: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<SymId>()));
    } break;
    case P_TYPE::COLOR: {
        Color color(data.value<Color>());
        XmlStreamWriter::writeElement(QString("%1 r=\"%2\" g=\"%3\" b=\"%4\" a=\"%5\"")
                                      .arg(name).arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha()));
    }
    break;
    case P_TYPE::ORNAMENT_STYLE: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<OrnamentStyle>()));
    } break;
    case P_TYPE::GLISS_STYLE: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<GlissandoStyle>()));
    } break;
    case P_TYPE::ALIGN: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<Align>()));
    }
    break;
    case P_TYPE::PLACEMENT_V: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<PlacementV>()));
    }
    break;
    case P_TYPE::PLACEMENT_H: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<PlacementH>()));
    }
    break;
    case P_TYPE::TEXT_PLACE: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<TextPlace>()));
    }
    break;
    case P_TYPE::DIRECTION_V: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<DirectionV>()));
    }
    break;
    case P_TYPE::DIRECTION_H: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<DirectionH>()));
    }
    break;
    case P_TYPE::ORIENTATION: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<Orientation>()));
    }
    break;
    // time
    case P_TYPE::FRACTION: {
        const Fraction& f = data.value<Fraction>();
        tag(name, f);
    }
    break;
    case P_TYPE::LAYOUTBREAK_TYPE: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<LayoutBreakType>()));
    } break;
    case P_TYPE::VELO_TYPE: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<VeloType>()));
    } break;
    case P_TYPE::BARLINE_TYPE: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<BarLineType>()));
    } break;
    case P_TYPE::NOTEHEAD_TYPE: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<NoteHeadType>()));
    } break;
    case P_TYPE::NOTEHEAD_SCHEME: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<NoteHeadScheme>()));
    } break;
    case P_TYPE::NOTEHEAD_GROUP: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<NoteHeadGroup>()));
    } break;
    case P_TYPE::CLEF_TYPE: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<ClefType>()));
    } break;
    case P_TYPE::DYNAMIC_TYPE: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<DynamicType>()));
    } break;
    case P_TYPE::DYNAMIC_RANGE: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<DynamicRange>()));
    } break;
    case P_TYPE::DYNAMIC_SPEED: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<DynamicSpeed>()));
    } break;
    case P_TYPE::HOOK_TYPE: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<HookType>()));
    } break;
    case P_TYPE::KEY_MODE: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<KeyMode>()));
    } break;
    case P_TYPE::TEXT_STYLE: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<TextStyleType>()));
    } break;
    case P_TYPE::CHANGE_METHOD: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<ChangeMethod>()));
    } break;
    case P_TYPE::ACCIDENTAL_ROLE: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<AccidentalRole>()));
    } break;
    case P_TYPE::PLAYTECH_TYPE: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<PlayingTechniqueType>()));
    } break;
    case P_TYPE::TEMPOCHANGE_TYPE: {
        XmlStreamWriter::writeElement(name, TConv::toXml(data.value<TempoChangeType>()));
    } break;
    default: {
        UNREACHABLE; //! TODO
    }
    break;
    }
}

#define IMPL_TAG(T) \
    void XmlWriter::tag(const QString& name, T val) \
    { \
        XmlStreamWriter::writeElement(name, val); \
    } \
    void XmlWriter::tag(const QString& name, T val, T def) \
    { \
        if (val == def) { \
            return; \
        } \
    \
        XmlStreamWriter::writeElement(name, val); \
    } \


IMPL_TAG(bool)
IMPL_TAG(int)
IMPL_TAG(double)
IMPL_TAG(const char*)

void XmlWriter::tag(const QString& name, const QString& val)
{
    XmlStreamWriter::writeElement(name, xmlString(val));
}

void XmlWriter::tag(const QString& name, const QString& val, const QString& def)
{
    if (val == def) {
        return;
    }

    XmlStreamWriter::writeElement(name, xmlString(val));
}

void XmlWriter::tag(const QString& name, const mu::PointF& p)
{
    XmlStreamWriter::writeElement(QString("%1 x=\"%2\" y=\"%3\"").arg(name).arg(p.x()).arg(p.y()));
}

void XmlWriter::tag(const char* name, const CustDef& cd)
{
    XmlStreamWriter::writeElement(QString("%1 degree=\"%2\" xAlt=\"%3\" octAlt=\"%4\"").arg(name).arg(cd.degree).arg(cd.xAlt).arg(cd.octAlt));
}

void XmlWriter::tag(const QString& name, const Fraction& v, const Fraction& def)
{
    if (v == def) {
        return;
    }

    XmlStreamWriter::writeElement(name, QString("%1/%2").arg(v.numerator()).arg(v.denominator()));
}

//---------------------------------------------------------
//   comment
//---------------------------------------------------------

void XmlWriter::comment(const QString& text)
{
    XmlStreamWriter::writeComment(text);
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
//   writeXml
//    string s is already escaped (& -> "&amp;")
//---------------------------------------------------------

void XmlWriter::writeXml(const QString& name, QString s)
{
    for (int i = 0; i < s.size(); ++i) {
        ushort c = s.at(i).unicode();
        if (c < 0x20 && c != 0x09 && c != 0x0A && c != 0x0D) {
            s[i] = '?';
        }
    }

    XmlStreamWriter::writeElement(name, s);
}

mu::engraving::WriteContext* XmlWriter::context() const
{
    if (!m_context) {
        m_context = new engraving::WriteContext();
        m_selfContext = true;
    }
    return m_context;
}

void XmlWriter::setContext(WriteContext* context)
{
    if (m_context && m_selfContext) {
        delete m_context;
    }

    m_context = context;
    m_selfContext = false;
}
}
