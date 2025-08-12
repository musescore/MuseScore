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
#include <chrono>
#include <iostream>

#include "global/types/string.h"
#ifdef SYSTEM_TINYXML
#include <tinyxml2.h>
#else
#include "thirdparty/tinyxml/tinyxml2.h"
#endif

#include "log.h"

using namespace muse;
using namespace muse::io;

struct XmlStreamReader::Xml {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLNode* node = nullptr;
    tinyxml2::XMLError err;
    String customErr;
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

#ifndef NO_QT_SUPPORT
XmlStreamReader::XmlStreamReader(const QByteArray& data)
{
    m_xml = new Xml();
    ByteArray ba = ByteArray::fromQByteArrayNoCopy(data);
    setData(ba);
}

#endif

XmlStreamReader::~XmlStreamReader()
{
    delete m_xml;
}

void XmlStreamReader::setData(const ByteArray& data_)
{
#ifndef NDEBUG
    struct Accumulator {
        double total_ms = 0.0;
        ~Accumulator() {
            LOGD() << "[XmlStreamReader] Total TINYXML2 parse time: "
                   << total_ms << " ms\n";
        }
    };
    static Accumulator acc;
#endif //NDEBUG

    auto start = std::chrono::steady_clock::now();
    m_xml->doc.Clear();
    m_xml->customErr.clear();
    m_token = TokenType::Invalid;

    if (data_.size() < 4) {
        m_xml->err = tinyxml2::XML_ERROR_EMPTY_DOCUMENT;
        LOGE() << m_xml->doc.ErrorIDToName(m_xml->err);
        return;
    }

    UtfCodec::Encoding enc = UtfCodec::xmlEncoding(data_);
    if (enc == UtfCodec::Encoding::Unknown) {
        m_xml->err = tinyxml2::XML_CAN_NOT_CONVERT_TEXT;
        LOGE() << "unknown encoding";
        return;
    }

    ByteArray data = data_; // no copy, implicit sharing
    if (enc == UtfCodec::Encoding::UTF_16LE) {
        String u16 = String::fromUtf16LE(data_);
        data = u16.toUtf8();
    } else if (enc == UtfCodec::Encoding::UTF_16BE) {
        String u16 = String::fromUtf16BE(data_);
        data = u16.toUtf8();
    }

    m_xml->err = m_xml->doc.Parse(reinterpret_cast<const char*>(data.constData()), data.size());

    if (m_xml->err == tinyxml2::XML_SUCCESS) {
        m_token = TokenType::NoToken;
    } else {
        LOGE() << m_xml->doc.ErrorIDToName(m_xml->err);
    }

#ifndef NDEBUG
    auto end = std::chrono::steady_clock::now();
    acc.total_ms += std::chrono::duration<double, std::milli>(end - start).count();
#endif //NDEBUG
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

static XmlStreamReader::TokenType resolveToken(tinyxml2::XMLNode* n, bool isStartElement)
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

static std::pair<tinyxml2::XMLNode*, XmlStreamReader::TokenType>
resolveNode(tinyxml2::XMLNode* currentNode, XmlStreamReader::TokenType currentToken)
{
    if (currentToken == XmlStreamReader::TokenType::StartElement) {
        tinyxml2::XMLNode* child = currentNode->FirstChild();
        if (child) {
            return { child, resolveToken(child, true) };
        }

        tinyxml2::XMLNode* sibling = currentNode->NextSibling();
        if (!sibling || sibling->ToElement() || sibling->ToText() || sibling->ToComment()) {
            return { currentNode, XmlStreamReader::TokenType::EndElement };
        }
    }

    tinyxml2::XMLNode* sibling = currentNode->NextSibling();
    if (sibling) {
        return { sibling, resolveToken(sibling, true) };
    }

    tinyxml2::XMLNode* parent = currentNode->Parent();
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

    if (m_xml->err != tinyxml2::XML_SUCCESS || m_token == EndDocument) {
        m_xml->node = nullptr;
        m_token = TokenType::Invalid;
        return m_token;
    }

    if (!m_xml->node) {
        m_xml->node = m_xml->doc.FirstChild();
        m_token = m_xml->node->ToDeclaration() ? TokenType::StartDocument : resolveToken(m_xml->node, true);
        return m_token;
    }

    std::pair<tinyxml2::XMLNode*, XmlStreamReader::TokenType> p = resolveNode(m_xml->node, m_token);

    m_xml->node = p.first;
    m_token = p.second;

    if (m_token == XmlStreamReader::TokenType::DTD) {
        tryParseEntity(m_xml);
    }

    return m_token;
}

void XmlStreamReader::tryParseEntity(Xml* xml)
{
    const char* nodeValue = xml->node->Value();

    if (!nodeValue || *nodeValue == '\0') {
        return;
    }

    const char* scanPos = nodeValue;
    while (true) {
        const char* entityPos = std::strstr(scanPos, "ENTITY");
        if (!entityPos) {
            break;
        }

        const char* cur = entityPos + 6; // after "ENTITY"

        // Skip whitespace
        while (*cur == ' ' || *cur == '\t' || *cur == '\r' || *cur == '\n') {
            ++cur;
        }

        // Optional leading '%' (parameter entity)
        if (*cur == '%') {
            ++cur;
            while (*cur == ' ' || *cur == '\t' || *cur == '\r' || *cur == '\n') {
                ++cur;
            }
        }

        // Name token: up to whitespace / quote / '>'
        const char* nameBegin = cur;
        while (*cur &&
               *cur != ' ' && *cur != '\t' && *cur != '\r' && *cur != '\n' &&
               *cur != '"' && *cur != '\'' && *cur != '>') {
            ++cur;
        }
        const char* nameEnd = cur;

        // Handle a leading '%' glued to the name (parameter entity without a space)
        if (nameBegin < nameEnd && *nameBegin == '%') {
            ++nameBegin;
            while (nameBegin < nameEnd &&
                   (*nameBegin == ' ' || *nameBegin == '\t' || *nameBegin == '\r' || *nameBegin == '\n')) {
                ++nameBegin;
            }
        }

        // Find first quote (either ' or ")
        while (*cur && *cur != '"' && *cur != '\'') {
            ++cur;
        }
        if (*cur != '"' && *cur != '\'') {
            scanPos = entityPos + 6;
            continue;
        }

        // Capture quoted value
        const char quoteChar = *cur++;
        const char* valueBegin = cur;
        while (*cur && *cur != quoteChar) {
            ++cur;
        }
        if (*cur != quoteChar) {
            scanPos = entityPos + 6;
            continue;
        }
        const char* valueEnd = cur;
        ++cur; // past closing quote

        // Build name/value:
        // - DO NOT remove "SYSTEM"/"PUBLIC" from the name (that was the over-stripping bug)
        // - Trim whitespace; if someone wrote "%NAME" without a space, strip the leading '%'
        std::string nameToken(nameBegin, static_cast<size_t>(nameEnd - nameBegin));
        String name = String::fromUtf8(nameToken.c_str()).trimmed();

        std::string valueToken(valueBegin, static_cast<size_t>(valueEnd - valueBegin));
        String value = String::fromUtf8(valueToken.c_str());

        if (!name.empty()) {
            m_entities[u'&' + name + u';'] = value;
        } else {
            LOGW() << "Ignoring malformed ENTITY: " << nodeValue;
        }

        scanPos = cur; // continue scanning for more ENTITY decls
    }
}

String XmlStreamReader::nodeValue(Xml* xml) const
{
    String str = String::fromUtf8(xml->node->Value());
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

AsciiStringView XmlStreamReader::tokenString() const
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
    return AsciiStringView();
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

AsciiStringView XmlStreamReader::name() const
{
    return (m_xml->node && m_xml->node->ToElement()) ? m_xml->node->Value() : AsciiStringView();
}

bool XmlStreamReader::hasAttribute(const char* name) const
{
    if (m_token != TokenType::StartElement) {
        return false;
    }

    tinyxml2::XMLElement* e = m_xml->node->ToElement();
    if (!e) {
        return false;
    }
    return e->FindAttribute(name) != nullptr;
}

String XmlStreamReader::attribute(const char* name) const
{
    if (m_token != TokenType::StartElement) {
        return String();
    }

    tinyxml2::XMLElement* e = m_xml->node->ToElement();
    if (!e) {
        return String();
    }
    return String::fromUtf8(e->Attribute(name));
}

String XmlStreamReader::attribute(const char* name, const String& def) const
{
    return hasAttribute(name) ? attribute(name) : def;
}

AsciiStringView XmlStreamReader::asciiAttribute(const char* name) const
{
    if (m_token != TokenType::StartElement) {
        return AsciiStringView();
    }

    tinyxml2::XMLElement* e = m_xml->node->ToElement();
    if (!e) {
        return AsciiStringView();
    }
    return e->Attribute(name);
}

AsciiStringView XmlStreamReader::asciiAttribute(const char* name, const AsciiStringView& def) const
{
    return hasAttribute(name) ? asciiAttribute(name) : def;
}

int XmlStreamReader::intAttribute(const char* name) const
{
    return asciiAttribute(name).toInt();
}

int XmlStreamReader::intAttribute(const char* name, int def) const
{
    return hasAttribute(name) ? intAttribute(name) : def;
}

double XmlStreamReader::doubleAttribute(const char* name) const
{
    return asciiAttribute(name).toDouble();
}

double XmlStreamReader::doubleAttribute(const char* name, double def) const
{
    return hasAttribute(name) ? doubleAttribute(name) : def;
}

std::vector<XmlStreamReader::Attribute> XmlStreamReader::attributes() const
{
    std::vector<Attribute> attrs;
    if (m_token != TokenType::StartElement) {
        return attrs;
    }

    tinyxml2::XMLElement* e = m_xml->node->ToElement();
    if (!e) {
        return attrs;
    }

    for (const tinyxml2::XMLAttribute* xa = e->FirstAttribute(); xa; xa = xa->Next()) {
        Attribute a;
        a.name = xa->Name();
        a.value = String::fromUtf8(xa->Value());
        attrs.push_back(std::move(a));
    }
    return attrs;
}

String XmlStreamReader::readBody() const
{
    if (m_xml->node) {
        tinyxml2::XMLPrinter printer;

        const tinyxml2::XMLElement* child = m_xml->node->FirstChildElement();
        while (child) {
            child->Accept(&printer);
            child = child->NextSiblingElement();
        }

        return String::fromStdString(printer.CStr());
    }
    return String();
}

String XmlStreamReader::text() const
{
    if (m_xml->node && (m_xml->node->ToText() || m_xml->node->ToComment())) {
        return nodeValue(m_xml);
    }
    return String();
}

AsciiStringView XmlStreamReader::asciiText() const
{
    if (m_xml->node && (m_xml->node->ToText() || m_xml->node->ToComment())) {
        return m_xml->node->Value();
    }
    return AsciiStringView();
}

String XmlStreamReader::readText()
{
    if (isStartElement()) {
        String result;
        while (1) {
            switch (readNext()) {
            case Characters:
                result = nodeValue(m_xml);
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
    return String();
}

AsciiStringView XmlStreamReader::readAsciiText()
{
    if (isStartElement()) {
        AsciiStringView result;
        while (1) {
            switch (readNext()) {
            case Characters:
                result = AsciiStringView(m_xml->node->Value());
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
    return AsciiStringView();
}

int XmlStreamReader::readInt(bool* ok, int base)
{
    AsciiStringView s = readAsciiText();
    return s.toInt(ok, base);
}

double XmlStreamReader::readDouble(bool* ok)
{
    AsciiStringView s = readAsciiText();
    return s.toDouble(ok);
}

int64_t XmlStreamReader::byteOffset() const
{
    // line number is a proxy for byte offset
    int64_t lineNum = m_xml->doc.ErrorLineNum();
    if (lineNum == 0 && m_xml->node) {
        lineNum = m_xml->node->GetLineNum();
    }
    return lineNum;
}

XmlStreamReader::Error XmlStreamReader::error() const
{
    if (!m_xml->customErr.isEmpty()) {
        return CustomError;
    }

    tinyxml2::XMLError err = m_xml->doc.ErrorID();
    if (err == tinyxml2::XML_SUCCESS) {
        return NoError;
    }

    return NotWellFormedError;
}

bool XmlStreamReader::isError() const
{
    return error() != NoError;
}

String XmlStreamReader::errorString() const
{
    if (!m_xml->customErr.empty()) {
        return m_xml->customErr;
    }
    return String::fromUtf8(m_xml->doc.ErrorStr());
}

void XmlStreamReader::raiseError(const String& message)
{
    m_xml->customErr = message;
}
