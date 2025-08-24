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
#pragma once

#include <string>
#include <string_view>
#include <variant>

#include "global/types/string.h"
#include "global/io/iodevice.h"

namespace muse {
class TextStream;
class XmlStreamWriter
{
public:
    XmlStreamWriter();
    explicit XmlStreamWriter(io::IODevice* dev);
    virtual ~XmlStreamWriter();

    XmlStreamWriter(const XmlStreamWriter&) = delete;
    XmlStreamWriter& operator=(const XmlStreamWriter&) = delete;

    using Value = std::variant<std::monostate, int, unsigned int, signed long int, unsigned long int, signed long long, unsigned long long,
                               double, const char*, AsciiStringView, String>;
    using Attribute = std::pair<AsciiStringView, Value>;
    using Attributes = std::vector<Attribute>;

    void setDevice(io::IODevice* dev);
    void flush();

    void startDocument();
    void writeDoctype(const String& type);

    void startElement(const AsciiStringView& name, const Attributes& attrs = {});
    void startElement(const String& name, const Attributes& attrs = {});
    void endElement();

    void element(const AsciiStringView& name, const Attributes& attrs = {});                // <element attr="value" />
    void element(const AsciiStringView& name, const Value& body);                           // <element>body</element>
    void element(const AsciiStringView& name, const Attributes& attrs, const Value& body);  // <element attr="value" >body</element>

    void comment(const String& text);

    static std::string escapeCodePoint(char32_t);
    static std::string escapeString(std::string_view);

protected:
    void startElementRaw(const String& name);
    void elementRaw(const String& nameWithAttributes, const Value& body);
    void elementStringRaw(const String& nameWithAttributes, const String& body);

private:

    void writeValue(const Value& v);

    struct Impl;
    Impl* m_impl = nullptr;
};
}
