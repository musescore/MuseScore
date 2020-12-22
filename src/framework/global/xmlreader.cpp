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

#include "xmlreader.h"

#include <QXmlStreamReader>
#include <QFile>

using namespace mu::framework;

static XmlReader::TokenType convertTokenType(QXmlStreamReader::TokenType type)
{
    switch (type) {
    case QXmlStreamReader::TokenType::NoToken:
    case QXmlStreamReader::TokenType::Invalid:
    case QXmlStreamReader::TokenType::DTD:
    case QXmlStreamReader::TokenType::EntityReference:
    case QXmlStreamReader::TokenType::ProcessingInstruction:
        return XmlReader::TokenType::Unknown;
    case QXmlStreamReader::TokenType::StartDocument:
        return XmlReader::TokenType::StartDocument;
    case QXmlStreamReader::TokenType::EndDocument:
        return XmlReader::TokenType::EndDocument;
    case QXmlStreamReader::TokenType::StartElement:
        return XmlReader::TokenType::StartElement;
    case QXmlStreamReader::TokenType::EndElement:
        return XmlReader::TokenType::EndElement;
    case QXmlStreamReader::TokenType::Comment:
        return XmlReader::TokenType::Comment;
    case QXmlStreamReader::TokenType::Characters:
        return XmlReader::TokenType::Characters;
    }

    return XmlReader::TokenType::Unknown;
}

XmlReader::XmlReader(const io::path& path)
{
    m_device = std::make_unique<QFile>(path.toQString());
    m_device->open(IODevice::ReadOnly);

    m_reader = std::make_unique<QXmlStreamReader>(m_device.get());
}

XmlReader::XmlReader(IODevice* device)
{
    m_reader = std::make_unique<QXmlStreamReader>(device);
}

XmlReader::XmlReader(const QByteArray& bytes)
{
    m_reader = std::make_unique<QXmlStreamReader>(bytes);
}

XmlReader::~XmlReader()
{
    if (m_device) {
        m_device->close();
    }
}

bool XmlReader::readNextStartElement()
{
    return m_reader->readNextStartElement();
}

XmlReader::TokenType XmlReader::readNext()
{
    return convertTokenType(m_reader->readNext());
}

XmlReader::TokenType XmlReader::tokenType() const
{
    return convertTokenType(m_reader->tokenType());
}

bool XmlReader::canRead() const
{
    return !m_reader->atEnd();
}

void XmlReader::skipCurrentElement()
{
    m_reader->skipCurrentElement();
}

std::string XmlReader::tagName() const
{
    return m_reader->name().toString().toStdString();
}

int XmlReader::intAttribute(std::string_view name, int defaultValue) const
{
    if (hasAttribute(name)) {
        return attributeValue(name).toInt();
    }

    return defaultValue;
}

double XmlReader::doubleAttribute(std::string_view name, double defaultValue) const
{
    if (hasAttribute(name)) {
        return attributeValue(name).toDouble();
    }

    return defaultValue;
}

std::string XmlReader::attribute(std::string_view name) const
{
    return attributeValue(name).toString().toStdString();
}

QStringRef XmlReader::attributeValue(std::string_view name) const
{
    return m_reader->attributes().value(name.data());
}

bool XmlReader::hasAttribute(std::string_view name) const
{
    return m_reader->attributes().hasAttribute(name.data());
}

int XmlReader::readInt()
{
    return readElementText().toInt();
}

double XmlReader::readDouble()
{
    return readElementText().toDouble();
}

std::string XmlReader::readString(ReadStringBehavior behavior)
{
    return readElementText(behavior).toStdString();
}

QString XmlReader::readElementText(ReadStringBehavior behavior)
{
    auto _behavior = QXmlStreamReader::ReadElementTextBehaviour::ErrorOnUnexpectedElement;

    switch (behavior) {
    case ErrorOnUnexpectedElement:
        _behavior = QXmlStreamReader::ErrorOnUnexpectedElement;
        break;
    case IncludeChildElements:
        _behavior = QXmlStreamReader::IncludeChildElements;
        break;
    case SkipChildElements:
        _behavior = QXmlStreamReader::SkipChildElements;
        break;
    }

    return m_reader->readElementText(_behavior);
}

bool XmlReader::success() const
{
    return !m_reader->hasError();
}

std::string XmlReader::error() const
{
    return m_reader->errorString().toStdString();
}
