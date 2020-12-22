//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "xmlwriter.h"

#include <QXmlStreamWriter>
#include <QFile>

using namespace mu::framework;

XmlWriter::XmlWriter(const io::path& path)
{
    m_device = std::make_unique<QFile>(path.toQString());
    m_device->open(IODevice::WriteOnly);

    initWriter(m_device.get());
}

XmlWriter::XmlWriter(IODevice* device)
{
    initWriter(device);
}

void XmlWriter::initWriter(IODevice* device)
{
    m_writer = std::make_unique<QXmlStreamWriter>(device);
    m_writer->setAutoFormatting(true);
}

XmlWriter::~XmlWriter()
{
    if (m_device) {
        m_device->close();
    }
}

void XmlWriter::writeStartDocument(std::string_view version)
{
    if (version.empty()) {
        m_writer->writeStartDocument();
        return;
    }

    m_writer->writeStartDocument(version.data());
}

void XmlWriter::writeEndDocument()
{
    m_writer->writeEndDocument();
}

void XmlWriter::writeStartElement(std::string_view name)
{
    m_writer->writeStartElement(name.data());
}

void XmlWriter::writeEndElement()
{
    m_writer->writeEndElement();
}

void XmlWriter::writeAttribute(std::string_view name, const std::string& value)
{
    m_writer->writeAttribute(name.data(), QString::fromStdString(value));
}

void XmlWriter::writeCharacters(const std::string& text)
{
    m_writer->writeCharacters(QString::fromStdString(text));
}

void XmlWriter::writeTextElement(std::string_view name, const std::string& text)
{
    m_writer->writeTextElement(name.data(), QString::fromStdString(text));
}

bool XmlWriter::success() const
{
    return !m_writer->hasError();
}
