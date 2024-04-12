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
#ifndef MU_ENGRAVING_XMLWRITER_H
#define MU_ENGRAVING_XMLWRITER_H

#include <map>
#include <unordered_map>

#include "io/iodevice.h"
#include "serialization/xmlstreamwriter.h"

#include "dom/property.h"

namespace mu::engraving {
class EngravingObject;
class XmlWriter : public muse::XmlStreamWriter
{
public:
    XmlWriter() = default;
    XmlWriter(muse::io::IODevice* dev);
    ~XmlWriter();

    const std::vector<std::pair<const EngravingObject*, AsciiStringView> >& elements() const { return _elements; }
    void setRecordElements(bool record) { _recordElements = record; }

    void startElementRaw(const String& name);
    void startElement(const AsciiStringView& name, const Attributes& attrs = {});
    void startElement(const EngravingObject* se, const Attributes& attrs = {});
    void startElement(const AsciiStringView& name, const EngravingObject* se, const Attributes& attrs = {});

    void tag(const AsciiStringView& name, const Attributes& attrs = {});
    void tag(const AsciiStringView& name, const Value& body);
    void tag(const AsciiStringView& name, const Value& val, const Value& def);
    void tag(const AsciiStringView& name, const Attributes& attrs, const Value& body);
    void tagRaw(const String& elementWithAttrs, const Value& body = Value());

    void tagProperty(Pid id, const PropertyValue& data, const PropertyValue& def = PropertyValue());
    void tagProperty(const AsciiStringView&, const PropertyValue& data, const PropertyValue& def = PropertyValue());

    void tagFraction(const AsciiStringView& name, const Fraction& v, const Fraction& def = Fraction());
    void tagPoint(const AsciiStringView& name, const PointF& v);

    void writeXml(const String&, String s);

    void comment(const String& text);

    static String xmlString(const String&);

private:
    void tagProperty(const AsciiStringView& name, P_TYPE type, const PropertyValue& data);

    std::vector<std::pair<const EngravingObject*, AsciiStringView> > _elements;
    bool _recordElements = false;
};
}

#endif // MU_ENGRAVING_XMLWRITER_H
