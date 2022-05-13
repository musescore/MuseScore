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
#include "xmlstreamreader.h"

#include <QXmlStreamReader>

using namespace mu;

XmlStreamReader::XmlStreamReader(QIODevice* device)
{
    m_reader = new QXmlStreamReader(device);
}

XmlStreamReader::XmlStreamReader(const QByteArray& data)
{
    m_reader = new QXmlStreamReader(data);
}

XmlStreamReader::~XmlStreamReader()
{
    delete m_reader;
}

void XmlStreamReader::setData(const QByteArray& data)
{
    m_reader->clear();
    m_reader->addData(data);
}

bool XmlStreamReader::readNextStartElement()
{
    return m_reader->readNextStartElement();
}

bool XmlStreamReader::atEnd() const
{
    return m_reader->atEnd();
}

XmlStreamReader::TokenType XmlStreamReader::readNext()
{
    return static_cast<TokenType>(m_reader->readNext());
}

XmlStreamReader::TokenType XmlStreamReader::tokenType() const
{
    return static_cast<TokenType>(m_reader->tokenType());
}

QString XmlStreamReader::tokenString() const
{
    return m_reader->tokenString();
}

bool XmlStreamReader::isWhitespace() const
{
    return m_reader->isWhitespace();
}

void XmlStreamReader::skipCurrentElement()
{
    m_reader->skipCurrentElement();
}

QStringRef XmlStreamReader::name() const
{
    return m_reader->name();
}

QString XmlStreamReader::attribute(const char* s) const
{
    return m_reader->attributes().value(s).toString();
}

bool XmlStreamReader::hasAttribute(const char* s) const
{
    return m_reader->attributes().hasAttribute(s);
}

std::vector<XmlStreamReader::Attribute> XmlStreamReader::attributes() const
{
    std::vector<Attribute> attrs;
    QXmlStreamAttributes qattrs = m_reader->attributes();
    attrs.reserve(qattrs.size());
    for (const QXmlStreamAttribute& qa : qattrs) {
        Attribute a;
        a.name = qa.name().toString();
        a.value = qa.value().toString();
        attrs.push_back(std::move(a));
    }
    return attrs;
}

QString XmlStreamReader::readElementText()
{
    return m_reader->readElementText();
}

QStringRef XmlStreamReader::text() const
{
    return m_reader->text();
}

int64_t XmlStreamReader::lineNumber() const
{
    return m_reader->lineNumber();
}

int64_t XmlStreamReader::columnNumber() const
{
    return m_reader->columnNumber();
}

XmlStreamReader::Error XmlStreamReader::error() const
{
    return static_cast<Error>(m_reader->error());
}

QString XmlStreamReader::errorString() const
{
    return m_reader->errorString();
}

void XmlStreamReader::raiseError(const QString& message)
{
    m_reader->raiseError(message);
}
