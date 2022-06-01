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
}

namespace Ms {
class XmlWriter : public mu::XmlStreamWriter
{
public:
    XmlWriter() = default;
    XmlWriter(mu::io::IODevice* dev);
    ~XmlWriter();

    const std::vector<std::pair<const EngravingObject*, QString> >& elements() const { return _elements; }
    void setRecordElements(bool record) { _recordElements = record; }

    void startObject(const QString&);
    void endObject();

    void startObject(const EngravingObject* se, const QString& attributes = QString());
    void startObject(const QString& name, const EngravingObject* se, const QString& attributes = QString());

    void tagE(const QString&);

    void tag(Pid id, const mu::engraving::PropertyValue& data, const mu::engraving::PropertyValue& def = mu::engraving::PropertyValue());
    void tagProperty(const mu::AsciiString&, const mu::engraving::PropertyValue& data,
                     const mu::engraving::PropertyValue& def = mu::engraving::PropertyValue());
    void tagProperty(const mu::AsciiString& name, mu::engraving::P_TYPE type, const mu::engraving::PropertyValue& data);

    void tag(const mu::AsciiString& name, const mu::AsciiString& v);
    void tag(const mu::AsciiString& name, const QString& v);
    void tag(const char* name, const QString& v) { tag(mu::AsciiString(name), v); }
    void tag(const char* name, const QString& v, const QString& d)
    {
        if (v == d) {
            return;
        }
        tag(mu::AsciiString(name), v);
    }

    void tag(const QString& name, const QString& val);
    void tag(const QString& name, int val);

    void tag(const mu::AsciiString& name, const Fraction& v, const Fraction& def = Fraction());
    void tag(const char* name, const CustDef& cd);

#define DECLARE_TAG(T) \
    void tag(const mu::AsciiString& name, T val); \
    void tag(const char* name, T val) { \
        tag(mu::AsciiString(name), val); \
    } \
    void tag(const mu::AsciiString& name, T val, T def); \
    void tag(const char* name,  T val, T def) { \
        tag(mu::AsciiString(name), val, def); \
    } \

    DECLARE_TAG(bool)
    DECLARE_TAG(int)
    DECLARE_TAG(double)
    DECLARE_TAG(const char*)

    void tag(const mu::AsciiString& name, const mu::PointF& v);

    void comment(const QString&);

    void writeXml(const QString&, QString s);

    mu::engraving::WriteContext* context() const;
    void setContext(mu::engraving::WriteContext* context);

    static QString xmlString(const QString&);
    static QString xmlString(ushort c);

private:
    std::vector<std::pair<const EngravingObject*, QString> > _elements;
    bool _recordElements = false;

    mutable mu::engraving::WriteContext* m_context = nullptr;
    mutable bool m_selfContext = false;
};
}

#endif // MU_ENGRAVING_XMLWRITER_H
