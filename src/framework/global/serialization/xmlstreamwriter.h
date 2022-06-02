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
#ifndef MU_GLOBAL_XMLSTREAMWRITER_H
#define MU_GLOBAL_XMLSTREAMWRITER_H

#include <list>
#include <variant>
#include <QString>

#include "types/string.h"
#include "io/iodevice.h"

namespace mu {
class TextStream;
class XmlStreamWriter
{
public:
    XmlStreamWriter();
    explicit XmlStreamWriter(io::IODevice* dev);
    virtual ~XmlStreamWriter();

    XmlStreamWriter(const XmlStreamWriter&) = delete;
    XmlStreamWriter& operator=(const XmlStreamWriter&) = delete;

    using Value = std::variant<int, int64_t, double, const char*, AsciiString, String>;
    using Attribute = std::pair<AsciiString, Value>;
    using Attributes = std::vector<Attribute>;

    void setDevice(io::IODevice* dev);
    void flush();

    void startDocument();
    void writeDoctype(const QString& type);

    void startElement(const AsciiString& name, const Attributes& attrs = {});
    void endElement();

    void writeStartElement(const QString& name);
    void writeStartElement(const QString& name, const QString& attributes);
    void writeEndElement();

    void writeElement(const QString& name, const QString& val);
    void writeElement(const QString& name, int val);

    void writeElement(const AsciiString& name, const char* val);
    void writeElement(const AsciiString& name, const AsciiString& val);
    void writeElement(const AsciiString& name, const QString& val);
    void writeElement(const AsciiString& name, int val);
    void writeElement(const AsciiString& name, int64_t val);
    void writeElement(const AsciiString& name, double val);

    void writeElement(const QString& nameWithAttributes);

    void writeComment(const QString& text);

private:

    void writeValue(const Value& v);

    struct Impl;
    Impl* m_impl = nullptr;
};
}

#endif // MU_GLOBAL_XMLSTREAMWRITER_H
