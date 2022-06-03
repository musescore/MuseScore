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
#ifndef MU_ENGRAVING_XMLWRITER_H
#define MU_ENGRAVING_XMLWRITER_H

#include <map>
#include <unordered_map>

#include "containers.h"
#include "io/iodevice.h"

#include "infrastructure/draw/color.h"
#include "libmscore/connector.h"
#include "libmscore/stafftype.h"
#include "libmscore/interval.h"
#include "libmscore/engravingitem.h"

#include "serialization/xmlstreamwriter.h"

namespace mu::engraving {
class WriteContext;
class XmlWriter : public XmlStreamWriter
{
public:
    XmlWriter() = default;
    XmlWriter(mu::io::IODevice* dev);
    ~XmlWriter();

    const std::vector<std::pair<const EngravingObject*, AsciiString> >& elements() const { return _elements; }
    void setRecordElements(bool record) { _recordElements = record; }

    void startElementRaw(const QString& name);
    void startElement(const AsciiString& name, const Attributes& attrs = {});
    void startElement(const EngravingObject* se, const Attributes& attrs = {});
    void startElement(const AsciiString& name, const EngravingObject* se, const Attributes& attrs = {});

    void tag(const AsciiString& name, const Attributes& attrs = {});
    void tag(const AsciiString& name, const Value& body);
    void tag(const AsciiString& name, const Value& val, const Value& def);
    void tag(const AsciiString& name, const Attributes& attrs, const Value& body);
    void tagRaw(const QString& elementWithAttrs, const Value& body = Value());

    void tagProperty(Pid id, const PropertyValue& data, const PropertyValue& def = PropertyValue());
    void tagProperty(const AsciiString&, const PropertyValue& data, const PropertyValue& def = PropertyValue());

    void tagFraction(const AsciiString& name, const Fraction& v, const Fraction& def = Fraction());
    void tagPoint(const AsciiString& name, const mu::PointF& v);

    void writeXml(const QString&, QString s);

    void comment(const String& text);

    WriteContext* context() const;
    void setContext(WriteContext* context);

    static QString xmlString(const QString&);

private:
    void tagProperty(const AsciiString& name, P_TYPE type, const PropertyValue& data);

    std::vector<std::pair<const EngravingObject*, AsciiString> > _elements;
    bool _recordElements = false;

    mutable WriteContext* m_context = nullptr;
    mutable bool m_selfContext = false;
};
}

#endif // MU_ENGRAVING_XMLWRITER_H
