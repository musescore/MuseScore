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

#include <cstring>

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

XmlStreamReader::XmlStreamReader()
{
    m_xml = new Xml();
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

XmlStreamReader::XmlStreamReader(const QByteArray& data)
{
    m_xml = new Xml();
    ByteArray ba = ByteArray::fromQByteArrayNoCopy(data);
    setData(ba);
}

XmlStreamReader::~XmlStreamReader()
{
    delete m_xml;
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
    } else if (n->ToUnknown()) {
        return XmlStreamReader::TokenType::DTD;
    }
    return XmlStreamReader::TokenType::Unknown;
}

static std::pair<XMLNode*, XmlStreamReader::TokenType> resolveNode(XMLNode* currentNode, XmlStreamReader::TokenType currentToken)
{
    if (currentToken == XmlStreamReader::TokenType::StartElement) {
        XMLNode* child = currentNode->FirstChild();
        if (child) {
            return { child, resolveToken(child, true) };
        }

        XMLNode* sibling = currentNode->NextSibling();
        if (!sibling || sibling->ToElement() || sibling->ToText()) {
            return { currentNode, XmlStreamReader::TokenType::EndElement };
        }
    }

    XMLNode* sibling = currentNode->NextSibling();
    if (sibling) {
        return { sibling, resolveToken(sibling, true) };
    }

    XMLNode* parent = currentNode->Parent();
    if (parent) {
        return { parent, resolveToken(parent, false) };
    }

    return { nullptr, XmlStreamReader::TokenType::EndDocument };
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

    std::pair<XMLNode*, XmlStreamReader::TokenType> p = resolveNode(m_xml->node, m_token);

    m_xml->node = p.first;
    m_token = p.second;

    if (m_token == XmlStreamReader::TokenType::DTD) {
        tryParseEntity(m_xml);
    }

    return m_token;
}

void XmlStreamReader::tryParseEntity(Xml* xml)
{
    static const char* ENTITY = { "ENTITY" };

    const char* str = xml->node->Value();
    if (std::strncmp(str, ENTITY, 6) == 0) {
        QString val(str);
        QStringList list = val.split(' ');
        if (list.length() == 3) {
            QString name = list.at(1);
            QString val = list.at(2);
            m_entities["&" + name + ";"] = val.mid(1, val.size() - 2);
        } else {
            LOGW() << "unknown ENTITY: " << val;
        }
    }
}

QString XmlStreamReader::nodeValue(Xml* xml) const
{
    QString str = xml->node->Value();
    if (!m_entities.empty()) {
        for (const auto& p : m_entities) {
            str.replace(p.first, p.second);
        }
    }
    return str;
}

XmlStreamReader::TokenType XmlStreamReader::tokenType() const
{
    return m_token;
}

AsciiString XmlStreamReader::tokenString() const
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
    case TokenType::DTD: return "DTD";
    case TokenType::Unknown: return "Unknown";
    }
    return AsciiString();
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

AsciiString XmlStreamReader::name() const
{
    return (m_xml->node && m_xml->node->ToElement()) ? m_xml->node->Value() : AsciiString();
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
                result.append(nodeValue(m_xml));
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
        return nodeValue(m_xml);
    }
    return QString();
}

AsciiString XmlStreamReader::readElementAsciiText()
{
    if (isStartElement()) {
        AsciiString result;
        while (1) {
            switch (readNext()) {
            case Characters:
                result = AsciiString(m_xml->node->Value());
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
    return AsciiString();
}

AsciiString XmlStreamReader::asciiText() const
{
    if (m_xml->node && (m_xml->node->ToText() || m_xml->node->ToComment())) {
        return m_xml->node->Value();
    }
    return AsciiString();
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

bool XmlStreamReader::isError() const
{
    return error() != NoError;
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
