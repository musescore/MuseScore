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

#include "thirdparty/tinyxml/tinyxml2.h"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace tinyxml2;

struct XmlStreamReader::Xml {
    XMLDocument doc;
    XMLNode* node = nullptr;
    XMLError err;
    QString customErr;
};

XmlStreamReader::XmlStreamReader(QIODevice* device)
{
    m_xml = new Xml();
    QByteArray data = device->readAll();
    ByteArray ba = ByteArray::fromQByteArrayNoCopy(data);
    setData(ba);
}

XmlStreamReader::XmlStreamReader(const QByteArray& data)
{
    m_xml = new Xml();
    ByteArray ba = ByteArray::fromQByteArrayNoCopy(data);
    setData(ba);
}

XmlStreamReader::XmlStreamReader(IODevice* device)
{
    m_xml = new Xml();
    ByteArray data = device->readAll();
    setData(data);
}

XmlStreamReader::XmlStreamReader(const ByteArray& data)
{
    m_xml = new Xml();
    setData(data);
}

XmlStreamReader::~XmlStreamReader()
{
    delete m_xml;
}

void XmlStreamReader::setData(const QByteArray& data)
{
    ByteArray ba = ByteArray::fromQByteArrayNoCopy(data);
    setData(ba);
}

void XmlStreamReader::setData(const ByteArray& data)
{
    m_xml->doc.Clear();
    m_xml->err = m_xml->doc.Parse(reinterpret_cast<const char*>(data.constData()), data.size());
    m_token = m_xml->err == XML_SUCCESS ? TokenType::NoToken : TokenType::Invalid;
    m_xml->customErr.clear();

    if (m_xml->err != XML_SUCCESS) {
        LOGE() << errorString();
    }
}

bool XmlStreamReader::readNextStartElement()
{
    while (readNext() != Invalid) {
        if (isEndElement()) {
            return false;
        } else if (isStartElement()) {
            return true;
        }
    }
    return false;
}

bool XmlStreamReader::atEnd() const
{
    return m_token == TokenType::EndDocument || m_token == TokenType::Invalid;
}

static XmlStreamReader::TokenType resolveToken(XMLNode* n, bool isStartElement)
{
    if (n->ToElement()) {
        return isStartElement ? XmlStreamReader::TokenType::StartElement : XmlStreamReader::TokenType::EndElement;
    } else if (n->ToText()) {
        return XmlStreamReader::TokenType::Characters;
    } else if (n->ToComment()) {
        return XmlStreamReader::TokenType::Comment;
    } else if (n->ToDeclaration()) {
        return XmlStreamReader::TokenType::StartDocument;
    } else if (n->ToDocument()) {
        return XmlStreamReader::TokenType::EndDocument;
    }
    return XmlStreamReader::TokenType::Unknown;
}

XmlStreamReader::TokenType XmlStreamReader::readNext()
{
    if (m_token == TokenType::Invalid) {
        return m_token;
    }

    if (m_xml->err != XML_SUCCESS || m_token == EndDocument) {
        m_xml->node = nullptr;
        m_token = TokenType::Invalid;
        return m_token;
    }

    if (!m_xml->node) {
        m_xml->node = m_xml->doc.FirstChild();
        m_token = m_xml->node->ToDeclaration() ? TokenType::StartDocument : resolveToken(m_xml->node, true);
        return m_token;
    }

    if (m_token == XmlStreamReader::TokenType::StartElement) {
        XMLNode* child = m_xml->node->FirstChild();
        if (child) {
            m_xml->node = child;
            m_token = resolveToken(child, true);
            return m_token;
        }

        XMLNode* sibling = m_xml->node->NextSibling();
        if (!sibling || sibling->ToElement() || sibling->ToText()) {
            m_token = XmlStreamReader::TokenType::EndElement;
            return m_token;
        }
    }

    XMLNode* sibling = m_xml->node->NextSibling();
    if (sibling) {
        m_xml->node = sibling;
        m_token = resolveToken(sibling, true);
        return m_token;
    }

    XMLNode* parent = m_xml->node->Parent();
    if (parent) {
        m_xml->node = parent;
        m_token = resolveToken(parent, false);
        return m_token;
    }

    m_xml->node = nullptr;
    m_token = XmlStreamReader::TokenType::EndDocument;
    return m_token;
}

XmlStreamReader::TokenType XmlStreamReader::tokenType() const
{
    return m_token;
}

QString XmlStreamReader::tokenString() const
{
    switch (m_token) {
    case TokenType::NoToken: return "NoToken";
    case TokenType::Invalid: return "Invalid";
    case TokenType::StartDocument: return "StartDocument";
    case TokenType::EndDocument: return "EndDocument";
    case TokenType::StartElement: return "StartElement";
    case TokenType::EndElement: return "EndElement";
    case TokenType::Characters: return "Characters";
    case TokenType::Comment: return "Comment";
    case TokenType::Unknown: return "Unknown";
    }
    return QString();
}

bool XmlStreamReader::isWhitespace() const
{
    return false;
}

void XmlStreamReader::skipCurrentElement()
{
    int depth = 1;
    while (depth && readNext() != Invalid) {
        if (isEndElement()) {
            --depth;
        } else if (isStartElement()) {
            ++depth;
        }
    }
}

QStringRef XmlStreamReader::name() const
{
    QString str = (m_xml->node && m_xml->node->ToElement()) ? m_xml->node->Value() : QString();
    m_stringRefs.push_back(std::move(str));
    return QStringRef(&m_stringRefs.back());
}

QString XmlStreamReader::attribute(const char* name) const
{
    if (m_token != TokenType::StartElement) {
        return QString();
    }

    XMLElement* e = m_xml->node->ToElement();
    if (!e) {
        return QString();
    }
    return e->Attribute(name);
}

bool XmlStreamReader::hasAttribute(const char* name) const
{
    if (m_token != TokenType::StartElement) {
        return false;
    }

    XMLElement* e = m_xml->node->ToElement();
    if (!e) {
        return false;
    }
    return e->FindAttribute(name) != nullptr;
}

std::vector<XmlStreamReader::Attribute> XmlStreamReader::attributes() const
{
    std::vector<Attribute> attrs;
    if (m_token != TokenType::StartElement) {
        return attrs;
    }

    XMLElement* e = m_xml->node->ToElement();
    if (!e) {
        return attrs;
    }

    for (const XMLAttribute* xa = e->FirstAttribute(); xa; xa = xa->Next()) {
        Attribute a;
        a.name = xa->Name();
        a.value = xa->Value();
        attrs.push_back(std::move(a));
    }
    return attrs;
}

QString XmlStreamReader::readElementText()
{
    if (isStartElement()) {
        QString result;
        while (1) {
            switch (readNext()) {
            case Characters:
                result.append(m_xml->node->Value());
                break;
            case EndElement:
                return result;
            case Comment:
                break;
            case StartElement:
                break;
            default:
                break;
            }
        }
    }
    return QString();
}

QString XmlStreamReader::text() const
{
    if (m_xml->node && (m_xml->node->ToText() || m_xml->node->ToComment())) {
        return m_xml->node->Value();
    }
    return QString();
}

int64_t XmlStreamReader::lineNumber() const
{
    return m_xml->doc.ErrorLineNum();
}

int64_t XmlStreamReader::columnNumber() const
{
    return 0;
}

XmlStreamReader::Error XmlStreamReader::error() const
{
    if (!m_xml->customErr.isEmpty()) {
        return CustomError;
    }

    XMLError err = m_xml->doc.ErrorID();
    if (err == XML_SUCCESS) {
        return NoError;
    }

    return NotWellFormedError;
}

QString XmlStreamReader::errorString() const
{
    if (!m_xml->customErr.isEmpty()) {
        return m_xml->customErr;
    }
    return m_xml->doc.ErrorStr();
}

void XmlStreamReader::raiseError(const QString& message)
{
    m_xml->customErr = !message.isEmpty() ? message : "CustomError";
}
