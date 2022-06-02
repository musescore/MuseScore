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

    template<typename T>
    void write(const QString& name, T val)
    {
        QString ename(QString(name).split(' ')[0]);
        putLevel();
        stream << '<' << name << '>';
        stream << val;
        stream << "</" << ename << '>' << '\n';
    }

    template<typename T>
    void write(const AsciiString& name, T val)
    {
        QString ename(QString(name.toQLatin1String()).split(' ')[0]);
        putLevel();
        stream << '<' << name << '>';
        stream << val;
        stream << "</" << ename << '>' << '\n';
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

void XmlStreamWriter::writeValue(const Value& v)
{
    //std::variant<int, int64_t, double, const char*, AsciiString, String>;
    switch (v.index()) {
    case 0: m_impl->stream << std::get<int>(v);
        break;
    case 1: m_impl->stream << std::get<int64_t>(v);
        break;
    case 2: m_impl->stream << std::get<double>(v);
        break;
    case 3: m_impl->stream << std::get<const char*>(v);
        break;
    case 4: m_impl->stream << std::get<AsciiString>(v);
        break;
    case 5: m_impl->stream << std::get<String>(v);
        break;
    default:
        LOGI() << "index: " << v.index();
        UNREACHABLE;
        break;
    }
}

void XmlStreamWriter::startElement(const AsciiString& name, const Attributes &attrs)
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

void XmlStreamWriter::endElement()
{
    m_impl->putLevel();
    m_impl->stream << "</" << mu::takeLast(m_impl->stack) << '>' << '\n';
    flush();
}

void XmlStreamWriter::writeStartElement(const QString& name)
{
//    IF_ASSERT_FAILED(!name.contains(" ")) {
//        int k = -1;
//    }

    m_impl->putLevel();
    m_impl->stream << '<' << name << '>' << '\n';
    m_impl->stack.push_back(name.split(' ')[0].toStdString());
}

void XmlStreamWriter::writeStartElement(const QString& name, const QString& attributes)
{
    m_impl->putLevel();
    m_impl->stream << '<' << name;
    if (!attributes.isEmpty()) {
        m_impl->stream << ' ' << attributes;
    }
    m_impl->stream << '>' << '\n';
    m_impl->stack.push_back(name.toStdString());
}

void XmlStreamWriter::writeEndElement()
{
    endElement();
}

void XmlStreamWriter::writeElement(const QString& name, const QString& val)
{
    m_impl->write<const QString&>(name, val);
}

void XmlStreamWriter::writeElement(const QString& name, int val)
{
    m_impl->write<int>(name, val);
}

void XmlStreamWriter::writeElement(const AsciiString& name, const char* val)
{
    m_impl->write<const char*>(name, val);
}

void XmlStreamWriter::writeElement(const AsciiString& name, const AsciiString& val)
{
    m_impl->write<const AsciiString&>(name, val);
}

void XmlStreamWriter::writeElement(const AsciiString& name, const QString& val)
{
    m_impl->write<const QString&>(name, val);
}

void XmlStreamWriter::writeElement(const AsciiString& name, int val)
{
    m_impl->write<int>(name, val);
}

void XmlStreamWriter::writeElement(const AsciiString& name, int64_t val)
{
    m_impl->write<int64_t>(name, val);
}

void XmlStreamWriter::writeElement(const AsciiString& name, double val)
{
    m_impl->write<double>(name, val);
}

void XmlStreamWriter::writeElement(const QString& nameWithAttributes)
{
    m_impl->putLevel();
    m_impl->stream << '<' << nameWithAttributes << "/>\n";
}

void XmlStreamWriter::writeComment(const QString& text)
{
    m_impl->putLevel();
    m_impl->stream << "<!-- " << text << " -->\n";
}
