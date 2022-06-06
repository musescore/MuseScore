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
#include "xmlstreamwriter.h"

#include "containers.h"
#include "textstream.h"

#include "log.h"

using namespace mu;

struct XmlStreamWriter::Impl {
    std::list<std::string> stack;
    TextStream stream;

    void putLevel()
    {
        size_t level = stack.size();
        for (size_t i = 0; i < level * 2; ++i) {
            stream << ' ';
        }
    }
};

XmlStreamWriter::XmlStreamWriter()
{
    m_impl = new Impl();
}

XmlStreamWriter::XmlStreamWriter(io::IODevice* dev)
{
    m_impl = new Impl();
    m_impl->stream.setDevice(dev);
}

XmlStreamWriter::~XmlStreamWriter()
{
    flush();
    delete m_impl;
}

void XmlStreamWriter::setDevice(io::IODevice* dev)
{
    m_impl->stream.setDevice(dev);
}

void XmlStreamWriter::flush()
{
    m_impl->stream.flush();
}

void XmlStreamWriter::startDocument()
{
    m_impl->stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
}

void XmlStreamWriter::writeDoctype(const QString& type)
{
    m_impl->stream << "<!DOCTYPE " << type << '>' << '\n';
}

QString XmlStreamWriter::escapeSymbol(ushort c)
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

QString XmlStreamWriter::escapeString(const QString& s)
{
    QString escaped;
    escaped.reserve(s.size());
    for (int i = 0; i < s.size(); ++i) {
        ushort c = s.at(i).unicode();
        escaped += escapeSymbol(c);
    }
    return escaped;
}

String XmlStreamWriter::escapeString(const String& s)
{
    //! TODO
    return String::fromQString(escapeString(s.toQString()));
}

String XmlStreamWriter::escapeString(const AsciiString& s)
{
    //! TODO
    return String::fromQString(escapeString(QString(s.ascii())));
}

void XmlStreamWriter::writeValue(const Value& v)
{
    // int, unsigned int, int64_t, size_t, double, const char*, AsciiString, String;
    switch (v.index()) {
    case 0:
        break;
    case 1: m_impl->stream << std::get<int>(v);
        break;
    case 2: m_impl->stream << std::get<unsigned int>(v);
        break;
    case 3: m_impl->stream << std::get<int64_t>(v);
        break;
    case 4: m_impl->stream << std::get<size_t>(v);
        break;
    case 5: m_impl->stream << std::get<double>(v);
        break;
    case 6: m_impl->stream << escapeString(AsciiString(std::get<const char*>(v)));
        break;
    case 7: m_impl->stream << escapeString(std::get<AsciiString>(v));
        break;
    case 8: m_impl->stream << escapeString(std::get<String>(v));
        break;
    default:
        LOGI() << "index: " << v.index();
        UNREACHABLE;
        break;
    }
}

void XmlStreamWriter::startElement(const AsciiString& name, const Attributes& attrs)
{
    IF_ASSERT_FAILED(!name.contains(' ')) {
    }

    m_impl->putLevel();
    m_impl->stream << '<' << name;
    for (const Attribute& a : attrs) {
        m_impl->stream << ' ' << a.first << '=' << '\"';
        writeValue(a.second);
        m_impl->stream << '\"';
    }
    m_impl->stream << '>' << '\n';
    m_impl->stack.push_back(name.ascii());
}

void XmlStreamWriter::startElement(const String& name, const Attributes& attrs)
{
    ByteArray ba = name.toAscii();
    startElement(AsciiString(ba.constChar(), ba.size()), attrs);
}

void XmlStreamWriter::startElementRaw(const QString& name)
{
    m_impl->putLevel();
    m_impl->stream << '<' << name << '>' << '\n';
    m_impl->stack.push_back(name.split(' ')[0].toStdString());
}

void XmlStreamWriter::endElement()
{
    m_impl->putLevel();
    m_impl->stream << "</" << mu::takeLast(m_impl->stack) << '>' << '\n';
    flush();
}

// <element attr="value" />
void XmlStreamWriter::element(const AsciiString& name, const Attributes& attrs)
{
    IF_ASSERT_FAILED(!name.contains(' ')) {
    }

    m_impl->putLevel();
    m_impl->stream << '<' << name;
    for (const Attribute& a : attrs) {
        m_impl->stream << ' ' << a.first << '=' << '\"';
        writeValue(a.second);
        m_impl->stream << '\"';
    }
    m_impl->stream << "/>\n";
}

void XmlStreamWriter::element(const AsciiString& name, const Value& body)
{
    IF_ASSERT_FAILED(!name.contains(' ')) {
    }

    m_impl->putLevel();
    m_impl->stream << '<' << name << '>';
    writeValue(body);
    m_impl->stream << "</" << name << '>' << '\n';
}

void XmlStreamWriter::element(const AsciiString& name, const Attributes& attrs, const Value& body)
{
    IF_ASSERT_FAILED(!name.contains(' ')) {
    }

    m_impl->putLevel();
    m_impl->stream << '<' << name;
    for (const Attribute& a : attrs) {
        m_impl->stream << ' ' << a.first << '=' << '\"';
        writeValue(a.second);
        m_impl->stream << '\"';
    }
    m_impl->stream << ">";
    writeValue(body);
    m_impl->stream << "</" << name << '>' << '\n';
}

void XmlStreamWriter::elementRaw(const QString& nameWithAttributes, const Value& body)
{
    m_impl->putLevel();
    if (body.index() == 0) {
        m_impl->stream << '<' << nameWithAttributes << "/>\n";
    } else {
        QString ename(QString(nameWithAttributes).split(' ')[0]);
        m_impl->stream << '<' << nameWithAttributes << '>';
        writeValue(body);
        m_impl->stream << "</" << ename << '>' << '\n';
    }
}

void XmlStreamWriter::elementStringRaw(const QString& nameWithAttributes, const QString& body)
{
    m_impl->putLevel();
    if (body.isEmpty()) {
        m_impl->stream << '<' << nameWithAttributes << "/>\n";
    } else {
        QString ename(QString(nameWithAttributes).split(' ')[0]);
        m_impl->stream << '<' << nameWithAttributes << '>';
        m_impl->stream << body;
        m_impl->stream << "</" << ename << '>' << '\n';
    }
}

void XmlStreamWriter::comment(const String& text)
{
    m_impl->putLevel();
    m_impl->stream << "<!-- " << text << " -->\n";
}
