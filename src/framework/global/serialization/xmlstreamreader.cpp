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
#include <sstream>
#include <chrono>
#include <iostream>

#include "pugixml.hpp"

#include "log.h"

using namespace muse;
using namespace muse::io;

struct XmlStreamReader::Xml {
    pugi::xml_document doc;
    pugi::xml_node node{};
    pugi::xml_parse_result result{};
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
        ~Accumulator()
        {
            LOGD() << "[XmlStreamReader] Total PUGI parse time: "
                   << total_ms << " ms\n";
        }
    };
    static Accumulator acc;

    auto start = std::chrono::steady_clock::now();
#endif //NDEBUG

    m_xml->doc.reset();
    m_xml->customErr.clear();
    m_token = TokenType::Invalid;

    if (data_.size() < 4) {
        m_xml->result = pugi::xml_parse_result{}; // zero it
        m_xml->result.status = pugi::status_no_document_element;
        m_xml->customErr = String(u"empty document");
        LOGE() << m_xml->customErr;
        return;
    }

    UtfCodec::Encoding enc = UtfCodec::xmlEncoding(data_);
    if (enc == UtfCodec::Encoding::Unknown) {
        m_xml->result = pugi::xml_parse_result{};
        m_xml->result.status = pugi::status_internal_error;
        m_xml->customErr = String(u"unknown encoding");
        LOGE() << m_xml->customErr;
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

    // pugi needs explicit flags to surface declaration/doctype/comments as nodes
    unsigned flags = pugi::parse_default
                     | pugi::parse_declaration
                     | pugi::parse_doctype
                     | pugi::parse_comments;
    m_xml->result = m_xml->doc.load_buffer(data.constData(), data.size(), flags);

    if (m_xml->result.status == pugi::status_ok) {
        m_token = TokenType::NoToken;
    } else {
        LOGE() << String::fromUtf8(m_xml->result.description());
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

static XmlStreamReader::TokenType resolveToken(pugi::xml_node n, bool isStartElement)
{
    switch (n.type()) {
    case pugi::node_element:
        return isStartElement
               ? XmlStreamReader::TokenType::StartElement
               : XmlStreamReader::TokenType::EndElement;

    case pugi::node_pcdata:
    case pugi::node_cdata:
        return XmlStreamReader::TokenType::Characters;

    case pugi::node_comment:
        return XmlStreamReader::TokenType::Comment;

    case pugi::node_declaration:   // <?xml ... ?>
        return XmlStreamReader::TokenType::StartDocument;

    case pugi::node_document:      // the document node
        return XmlStreamReader::TokenType::EndDocument;

    case pugi::node_doctype:       // <!DOCTYPE ...>
        return XmlStreamReader::TokenType::DTD;

    default:                       // includes node_pi, node_null, etc.
        return XmlStreamReader::TokenType::Unknown;
    }
}

static std::pair<pugi::xml_node, XmlStreamReader::TokenType>
resolveNode(pugi::xml_node currentNode, XmlStreamReader::TokenType currentToken)
{
    if (currentToken == XmlStreamReader::TokenType::StartElement) {
        pugi::xml_node child = currentNode.first_child();
        if (child) {
            return { child, resolveToken(child, /*isStartElement=*/ true) };
        }

        pugi::xml_node sibling = currentNode.next_sibling();
        if (!sibling
            || sibling.type() == pugi::node_element
            || sibling.type() == pugi::node_pcdata
            || sibling.type() == pugi::node_cdata
            || sibling.type() == pugi::node_comment) {
            // Mirror tiny: close the element before stepping to "content" siblings
            return { currentNode, XmlStreamReader::TokenType::EndElement };
        }
        // else: fall through to handle non-content siblings (doctype, declaration, etc.)
    }

    pugi::xml_node sibling = currentNode.next_sibling();
    if (sibling) {
        return { sibling, resolveToken(sibling, /*isStartElement=*/ true) };
    }

    pugi::xml_node parent = currentNode.parent();
    if (parent) {
        return { parent, resolveToken(parent, /*isStartElement=*/ false) };
    }

    return { pugi::xml_node(), XmlStreamReader::TokenType::EndDocument };
}

XmlStreamReader::TokenType XmlStreamReader::readNext()
{
    if (m_token == TokenType::Invalid) {
        return m_token;
    }

    if (m_xml->result.status != pugi::status_ok || m_token == TokenType::EndDocument) {
        m_xml->node = pugi::xml_node();
        m_token = TokenType::Invalid;
        return m_token;
    }

    if (!m_xml->node) {
        m_xml->node = m_xml->doc.first_child();
        if (!m_xml->node) {
            // Empty doc — treat as end
            m_token = TokenType::EndDocument;
            return m_token;
        }
        m_token = (m_xml->node.type() == pugi::node_declaration)
                  ? TokenType::StartDocument
                  : resolveToken(m_xml->node, /*isStartElement=*/ true);
        return m_token;
    }

    std::pair<pugi::xml_node, XmlStreamReader::TokenType> p = resolveNode(m_xml->node, m_token);

    m_xml->node = p.first;
    m_token = p.second;

    if (m_token == TokenType::DTD) {
        tryParseEntity(m_xml);
    }

    return m_token;
}

void XmlStreamReader::tryParseEntity(Xml* xml)
{
    const char* nodeValue = xml->node.value();

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
        while (*cur
               && *cur != ' ' && *cur != '\t' && *cur != '\r' && *cur != '\n'
               && *cur != '"' && *cur != '\'' && *cur != '>') {
            ++cur;
        }
        const char* nameEnd = cur;

        // Handle a leading '%' glued to the name (parameter entity without a space)
        if (nameBegin < nameEnd && *nameBegin == '%') {
            ++nameBegin;
            while (nameBegin < nameEnd
                   && (*nameBegin == ' ' || *nameBegin == '\t' || *nameBegin == '\r' || *nameBegin == '\n')) {
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
        // - DO NOT remove "SYSTEM"/"PUBLIC" from the name
        // - Trim whitespace
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

// emulate tinyxml2::XMLNode::Value
String XmlStreamReader::nodeValue(Xml* xml) const
{
    const pugi::xml_node n = xml->node;

    const char* raw = "";
    switch (n.type()) {
    case pugi::node_element:
    case pugi::node_pi:
    case pugi::node_declaration:
    case pugi::node_doctype:
    case pugi::node_document: // usually empty
        raw = n.name();
        break;

    case pugi::node_pcdata:
    case pugi::node_cdata:
    case pugi::node_comment:
        raw = n.value();
        break;

    default:
        raw = "";
        break;
    }

    String str = String::fromUtf8(raw);
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
    return (m_xml->node && m_xml->node.type() == pugi::node_element)
           ? AsciiStringView(m_xml->node.name())
           : AsciiStringView();
}

bool XmlStreamReader::hasAttribute(const char* name) const
{
    if (m_token != TokenType::StartElement) {
        return false;
    }

    if (!m_xml->node || m_xml->node.type() != pugi::node_element) {
        return false;
    }

    return m_xml->node.attribute(name);
}

String XmlStreamReader::attribute(const char* name) const
{
    if (m_token != TokenType::StartElement) {
        return String();
    }

    if (!m_xml->node || m_xml->node.type() != pugi::node_element) {
        return String();
    }

    pugi::xml_attribute attr = m_xml->node.attribute(name);
    if (!attr) {
        return String();
    }

    return String::fromUtf8(attr.value());
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

    if (!m_xml->node || m_xml->node.type() != pugi::node_element) {
        return AsciiStringView();
    }

    pugi::xml_attribute attr = m_xml->node.attribute(name);
    return attr ? AsciiStringView(attr.value()) : AsciiStringView();
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

    if (!m_xml->node || m_xml->node.type() != pugi::node_element) {
        return attrs;
    }

    for (pugi::xml_attribute xa = m_xml->node.first_attribute(); xa; xa = xa.next_attribute()) {
        Attribute a;
        a.name = xa.name();
        a.value = String::fromUtf8(xa.value());
        attrs.push_back(std::move(a));
    }

    return attrs;
}

String XmlStreamReader::readBody() const
{
    if (!m_xml->node) {
        return String();
    }

    std::ostringstream oss;

    for (pugi::xml_node child = m_xml->node.first_child();
         child;
         child = child.next_sibling()) {
        if (child.type() == pugi::node_element) {
            // Match tinyxml2::XMLPrinter default (no indentation/line breaks)
            child.print(oss, "", pugi::format_raw);
        }
    }

    return String::fromStdString(oss.str());
}

String XmlStreamReader::text() const
{
    if (m_xml->node) {
        pugi::xml_node_type t = m_xml->node.type();
        if (t == pugi::node_pcdata || t == pugi::node_cdata || t == pugi::node_comment) {
            return nodeValue(m_xml);
        }
    }
    return String();
}

AsciiStringView XmlStreamReader::asciiText() const
{
    if (m_xml->node) {
        pugi::xml_node_type t = m_xml->node.type();
        if (t == pugi::node_pcdata || t == pugi::node_cdata || t == pugi::node_comment) {
            return m_xml->node.value();
        }
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
                result = AsciiStringView(m_xml->node.value());
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
    if (!m_xml->node) {
        return 0;
    }

    // Use byte offset from start of document as a temporary proxy for line number
    return static_cast<int64_t>(m_xml->node.offset_debug());
}

XmlStreamReader::Error XmlStreamReader::error() const
{
    if (!m_xml->customErr.isEmpty()) {
        return CustomError;
    }

    if (m_xml->result.status == pugi::status_ok) {
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
    return String::fromUtf8(m_xml->result.description());
}

void XmlStreamReader::raiseError(const String& message)
{
    m_xml->customErr = message;
}
