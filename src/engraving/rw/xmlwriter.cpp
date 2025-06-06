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

#include "types/typesconv.h"

#include "dom/engravingitem.h"
#include "dom/property.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
XmlWriter::XmlWriter(muse::io::IODevice* device)
    : XmlStreamWriter(device)
{
}

XmlWriter::~XmlWriter()
{
}

void XmlWriter::startElementRaw(const String& s)
{
    XmlStreamWriter::startElementRaw(s);
}

void XmlWriter::startElement(const AsciiStringView& name, const Attributes& attrs)
{
    XmlStreamWriter::startElement(name, attrs);
}

void XmlWriter::startElement(const EngravingObject* se, const Attributes& attrs)
{
    startElement(se->typeName(), se, attrs);
}

void XmlWriter::startElement(const AsciiStringView& name, const EngravingObject* se, const Attributes& attrs)
{
    XmlStreamWriter::startElement(name, attrs);

    if (_recordElements) {
        _elements.emplace_back(se, name);
    }
}

void XmlWriter::tag(const AsciiStringView& name, const Attributes& attrs)
{
    XmlStreamWriter::element(name, attrs);
}

void XmlWriter::tag(const AsciiStringView& name, const Value& body)
{
    XmlStreamWriter::element(name, body);
}

void XmlWriter::tag(const AsciiStringView& name, const Value& val, const Value& def)
{
    if (val == def) {
        return;
    }
    tag(name, val);
}

void XmlWriter::tag(const AsciiStringView& name, const Attributes& attrs, const Value& body)
{
    XmlStreamWriter::element(name, attrs, body);
}

void XmlWriter::tagRaw(const String& elementWithAttrs, const Value& body)
{
    XmlStreamWriter::elementRaw(elementWithAttrs, body);
}

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void XmlWriter::tagProperty(Pid id, const PropertyValue& val, const PropertyValue& def)
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

    const String writableVal(propertyToString(id, val, /* mscx */ true));
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

void XmlWriter::tagProperty(const AsciiStringView& name, const PropertyValue& val, const PropertyValue& def)
{
    if (val == def) {
        return;
    }

    tagProperty(name, val.type(), val);
}

void XmlWriter::tagProperty(const AsciiStringView& name, P_TYPE type, const PropertyValue& data)
{
    switch (type) {
    case P_TYPE::UNDEFINED:
        UNREACHABLE;
        break;
    // base
    case P_TYPE::BOOL:
        element(name, int(data.value<bool>()));
        break;
    case P_TYPE::INT:
        element(name, data.value<int>());
        break;
    case P_TYPE::REAL:
        element(name, data.value<double>());
        break;
    case P_TYPE::STRING:
        element(name, data.value<String>());
        break;
    // geometry
    case P_TYPE::POINT: {
        PointF p = data.value<PointF>();
        element(name, { { "x", p.x() }, { "y", p.y() } });
    }
    break;
    case P_TYPE::SIZE: {
        SizeF s = data.value<SizeF>();
        element(name, { { "w", s.width() }, { "h", s.height() } });
    }
    break;
    case P_TYPE::DRAW_PATH:
        UNREACHABLE; //! TODO
        break;
    case P_TYPE::SCALE: {
        ScaleF s = data.value<ScaleF>();
        element(name, { { "w", s.width() }, { "h", s.height() } });
    } break;
    case P_TYPE::SPATIUM:
        element(name, data.value<Spatium>().val());
        break;
    case P_TYPE::MILLIMETRE:
        element(name, data.value<Millimetre>().val());
        break;

    // draw
    case P_TYPE::SYMID: {
        element(name, TConv::toXml(data.value<SymId>()));
    } break;
    case P_TYPE::COLOR: {
        Color color(data.value<Color>());
        element(name, { { "r", color.red() }, { "g", color.green() }, { "b", color.blue() }, { "a", color.alpha() } });
    }
    break;
    case P_TYPE::ORNAMENT_STYLE: {
        element(name, TConv::toXml(data.value<OrnamentStyle>()));
    } break;
    case P_TYPE::GLISS_STYLE: {
        element(name, TConv::toXml(data.value<GlissandoStyle>()));
    } break;
    case P_TYPE::GLISS_TYPE: {
        element(name, TConv::toXml(data.value<GlissandoType>()));
    } break;
    case P_TYPE::ALIGN: {
        element(name, TConv::toXml(data.value<Align>()));
    }
    break;
    case P_TYPE::ALIGN_H: {
        element(name, TConv::toXml(data.value<AlignH>()));
    }
    break;
    case P_TYPE::PLACEMENT_V: {
        element(name, TConv::toXml(data.value<PlacementV>()));
    }
    break;
    case P_TYPE::PLACEMENT_H: {
        element(name, TConv::toXml(data.value<PlacementH>()));
    }
    break;
    case P_TYPE::TEXT_PLACE: {
        element(name, TConv::toXml(data.value<TextPlace>()));
    }
    break;
    case P_TYPE::DIRECTION_V: {
        element(name, TConv::toXml(data.value<DirectionV>()));
    }
    break;
    case P_TYPE::DIRECTION_H: {
        element(name, TConv::toXml(data.value<DirectionH>()));
    }
    break;
    case P_TYPE::ORIENTATION: {
        element(name, TConv::toXml(data.value<Orientation>()));
    }
    break;
    // time
    case P_TYPE::FRACTION: {
        tagFraction(name, data.value<Fraction>());
    }
    break;
    case P_TYPE::LAYOUTBREAK_TYPE: {
        element(name, TConv::toXml(data.value<LayoutBreakType>()));
    } break;
    case P_TYPE::BARLINE_TYPE: {
        element(name, TConv::toXml(data.value<BarLineType>()));
    } break;
    case P_TYPE::NOTEHEAD_TYPE: {
        element(name, TConv::toXml(data.value<NoteHeadType>()));
    } break;
    case P_TYPE::NOTEHEAD_SCHEME: {
        element(name, TConv::toXml(data.value<NoteHeadScheme>()));
    } break;
    case P_TYPE::NOTEHEAD_GROUP: {
        element(name, TConv::toXml(data.value<NoteHeadGroup>()));
    } break;
    case P_TYPE::CLEF_TYPE: {
        element(name, TConv::toXml(data.value<ClefType>()));
    } break;
    case P_TYPE::DYNAMIC_TYPE: {
        element(name, TConv::toXml(data.value<DynamicType>()));
    } break;
    case P_TYPE::DYNAMIC_SPEED: {
        element(name, TConv::toXml(data.value<DynamicSpeed>()));
    } break;
    case P_TYPE::LINE_TYPE: {
        element(name, TConv::toXml(data.value<LineType>()));
    } break;
    case P_TYPE::HOOK_TYPE: {
        element(name, TConv::toXml(data.value<HookType>()));
    } break;
    case P_TYPE::KEY_MODE: {
        element(name, TConv::toXml(data.value<KeyMode>()));
    } break;
    case P_TYPE::TEXT_STYLE: {
        element(name, TConv::toXml(data.value<TextStyleType>()));
    } break;
    case P_TYPE::CHANGE_METHOD: {
        element(name, TConv::toXml(data.value<ChangeMethod>()));
    } break;
    case P_TYPE::ACCIDENTAL_ROLE: {
        element(name, TConv::toXml(data.value<AccidentalRole>()));
    } break;
    case P_TYPE::PLAYTECH_TYPE: {
        element(name, TConv::toXml(data.value<PlayingTechniqueType>()));
    } break;
    case P_TYPE::TEMPOCHANGE_TYPE: {
        element(name, TConv::toXml(data.value<GradualTempoChangeType>()));
    } break;
    case P_TYPE::ORNAMENT_INTERVAL: {
        element(name, TConv::toXml(data.value<OrnamentInterval>()));
    } break;
    case P_TYPE::TIE_PLACEMENT: {
        element(name, TConv::toXml(data.value<TiePlacement>()));
    } break;
    case P_TYPE::VOICE_ASSIGNMENT: {
        element(name, TConv::toXml(data.value<VoiceAssignment>()));
    } break;
    case P_TYPE::AUTO_ON_OFF: {
        element(name, TConv::toXml(data.value<AutoOnOff>()));
    } break;
    case P_TYPE::INT_VEC: {
        element(name, TConv::toXml(data.value<std::vector<int> >()));
    } break;
    case P_TYPE::PARTIAL_SPANNER_DIRECTION: {
        element(name, TConv::toXml(data.value<PartialSpannerDirection>()));
    } break;
    default: {
        UNREACHABLE; //! TODO
    }
    break;
    }
}

void XmlWriter::tagPoint(const AsciiStringView& name, const PointF& p)
{
    tag(name, { { "x", p.x() }, { "y", p.y() } });
}

void XmlWriter::tagFraction(const AsciiStringView& name, const Fraction& v, const Fraction& def)
{
    if (v == def) {
        return;
    }

    element(name, v.toString());
}

void XmlWriter::writeXml(const String& name, String s)
{
    for (size_t i = 0; i < s.size(); ++i) {
        char16_t c = s.at(i).unicode();
        if (c < 0x20 && c != 0x09 && c != 0x0A && c != 0x0D) {
            s[i] = u'?';
        }
    }

    elementStringRaw(name, s);
}

void XmlWriter::comment(const String& text)
{
    XmlStreamWriter::comment(text);
}

String XmlWriter::xmlString(const String& s)
{
    return XmlStreamWriter::escapeString(s);
}
}
