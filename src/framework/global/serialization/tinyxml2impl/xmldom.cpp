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
#include "xmldom.h"

#include <chrono>
#include <iostream>

#ifdef SYSTEM_TINYXML
#include <tinyxml2.h>
#else
#include "thirdparty/tinyxml/tinyxml2.h"
#endif

#include "log.h"

using namespace muse;

struct muse::XmlDomImplData
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err = tinyxml2::XML_SUCCESS;
};

using xml_node_impl = const tinyxml2::XMLNode*;
using xml_element_impl = const tinyxml2::XMLElement*;
using xml_attr_impl = const tinyxml2::XMLAttribute*;

// ================================================
// generic pack/unpack
// ================================================

namespace {
template <class T>
inline xml_handle pack_handle(const T& t) noexcept {
    static_assert(std::is_trivially_copyable_v<T>,
                  "Backend handle must be trivially copyable");
    static_assert(sizeof(T) <= sizeof(xml_handle),
                  "Increase xml_handle slots/size");
    xml_handle h{};                   // zero both slots
    std::memcpy(&h, &t, sizeof(T));  // copy only the bytes T needs
    return h;
}

template <class T>
inline T unpack_handle(xml_handle h) noexcept {
    static_assert(std::is_trivially_copyable_v<T>,
                  "Backend handle must be trivially copyable");
    static_assert(sizeof(T) <= sizeof(xml_handle),
                  "Increase xml_handle slots/size");
    T t{};                            // zero-init destination
    std::memcpy(&t, &h, sizeof(T));   // copy back only sizeof(T)
    return t;
}
} // anonymous namespace

// ================================================
// XmlDomNode
// ================================================


XmlDomNode::XmlDomNode(const std::shared_ptr<XmlDomImplData>& xml, xml_node_handle node)
    : m_xml(xml), m_node(node)
{
}

bool XmlDomNode::isNull() const
{
    return !static_cast<bool>(m_node);
}

String XmlDomNode::nodeName() const
{
    return m_node ? String::fromUtf8(unpack_handle<xml_node_impl>(m_node)->Value()) : String();
}

bool XmlDomNode::hasChildNodes() const
{
    return m_node ? !unpack_handle<xml_node_impl>(m_node)->NoChildren() : false;
}

XmlDomNode XmlDomNode::firstChild() const
{
    if (!m_node) {
        return XmlDomNode(m_xml, xml_node_handle());
    }
    xml_node_impl n = unpack_handle<xml_node_impl>(m_node)->FirstChild();
    return XmlDomNode(m_xml, pack_handle(n));
}

XmlDomElement XmlDomNode::firstChildElement(const char* name) const
{
    if (!m_node) {
        return XmlDomElement(m_xml, xml_node_handle());
    }
    xml_element_impl e = unpack_handle<xml_node_impl>(m_node)->FirstChildElement(name);
    return XmlDomElement(m_xml, pack_handle(static_cast<xml_node_impl>(e)));
}

XmlDomNode XmlDomNode::nextSibling() const
{
    if (!m_node) {
        return XmlDomNode(m_xml, xml_node_handle());
    }
    xml_node_impl n = unpack_handle<xml_node_impl>(m_node)->NextSibling();
    return XmlDomNode(m_xml, pack_handle(n));
}

XmlDomNode XmlDomNode::previousSibling() const
{
    if (!m_node) {
        return XmlDomNode(m_xml, xml_node_handle());
    }
    xml_node_impl n = unpack_handle<xml_node_impl>(m_node)->PreviousSibling();
    return XmlDomNode(m_xml, pack_handle(n));
}

XmlDomNode XmlDomNode::parent() const
{
    if (!m_node) {
        return XmlDomNode(m_xml, xml_node_handle());
    }
    xml_node_impl n = unpack_handle<xml_node_impl>(m_node)->Parent();
    return XmlDomNode(m_xml, pack_handle(n));
}

XmlDomElement XmlDomNode::nextSiblingElement(const char* name) const
{
    if (!m_node) {
        return XmlDomElement(m_xml, xml_node_handle());
    }
    xml_element_impl e = unpack_handle<xml_node_impl>(m_node)->NextSiblingElement(name);
    return XmlDomElement(m_xml, pack_handle(static_cast<xml_node_impl>(e)));
}

XmlDomElement XmlDomNode::previousSiblingElement(const char* name) const
{
    if (!m_node) {
        return XmlDomElement(m_xml, xml_node_handle());
    }
    xml_element_impl e = unpack_handle<xml_node_impl>(m_node)->PreviousSiblingElement(name);
    return XmlDomElement(m_xml, pack_handle(static_cast<xml_node_impl>(e)));
}

XmlDomElement XmlDomNode::toElement() const
{
    if (!m_node) {
        return XmlDomElement(m_xml, xml_node_handle());
    }
    xml_element_impl e = unpack_handle<xml_node_impl>(m_node)->ToElement();
    return XmlDomElement(m_xml, pack_handle(static_cast<xml_node_impl>(e)));
}

// ================================================
// XmlDomAttribute
// ================================================

XmlDomAttribute::XmlDomAttribute(const std::shared_ptr<XmlDomImplData>& data, xml_attr_handle attribute)
    : m_xml(data), m_attribute(attribute)
{
}

bool XmlDomAttribute::isNull() const
{
    return !static_cast<bool>(m_attribute);
}

String XmlDomAttribute::attributeName() const
{
    if (!m_attribute) {
        return String();
    }
    xml_attr_impl a = unpack_handle<xml_attr_impl>(m_attribute);
    return String::fromUtf8(a->Name());
}

String XmlDomAttribute::value() const
{
    if (!m_attribute) {
        return String();
    }
    xml_attr_impl a = unpack_handle<xml_attr_impl>(m_attribute);
    return String::fromUtf8(a->Value());
}

XmlDomAttribute XmlDomAttribute::nextAttribute() const
{
    if (!m_attribute) {
        return XmlDomAttribute(m_xml, xml_attr_handle());
    }
    xml_attr_impl a = unpack_handle<xml_attr_impl>(m_attribute);
    return XmlDomAttribute(m_xml, pack_handle(a->Next()));
}

// ================================================
// XmlDomElement
// ================================================

XmlDomElement::XmlDomElement(const std::shared_ptr<XmlDomImplData>& data, xml_node_handle node)
    : XmlDomNode(data, node)
{
}

String XmlDomElement::text() const
{
    xml_element_impl e = unpack_handle<xml_node_impl>(m_node)->ToElement();
    if (!e) {
        return String();
    }

    String result;
    for (xml_node_impl n = e->FirstChild(); n != nullptr; n = n->NextSibling()) {
        const tinyxml2::XMLText* t = n->ToText();
        if (t) {
            result += String::fromUtf8(t->Value());
        }
    }

    return result;
}

XmlDomAttribute XmlDomElement::firstAttribute() const
{
    if (m_node) {
        if (xml_element_impl e = unpack_handle<xml_node_impl>(m_node)->ToElement()) {
            return XmlDomAttribute(m_xml, pack_handle(e->FirstAttribute()));
        }
    }
    return XmlDomAttribute(m_xml, xml_attr_handle());
}

XmlDomAttribute XmlDomElement::attribute(const char* name) const
{
    if (m_node) {
        if (xml_element_impl e = unpack_handle<xml_node_impl>(m_node)->ToElement()) {
            return XmlDomAttribute(m_xml, pack_handle(e->FindAttribute(name)));
        }
    }
    return XmlDomAttribute(m_xml, xml_attr_handle());
}

// ================================================
// XmlDomDocument
// ================================================

XmlDomDocument::XmlDomDocument()
{
    m_xml = std::make_shared<XmlDomImplData>();
}

void XmlDomDocument::setContent(const ByteArray& data)
{
#ifndef NDEBUG
    struct Accumulator {
        double total_ms = 0.0;
        size_t count = 0;
        ~Accumulator() {
            LOGD() << "[XmlDom TINYXML2] Parsed " << count << " docs in "
                   << total_ms << " ms (avg "
                   << (count ? total_ms / count : 0.0) << " ms/doc)\n";
        }
    };
    static Accumulator acc;

    auto start = std::chrono::steady_clock::now();
#endif //NDEBUG

    m_xml->doc.Clear();
    m_xml->err = m_xml->doc.Parse(reinterpret_cast<const char*>(data.constData()), data.size());

    if (m_xml->err != tinyxml2::XML_SUCCESS) {
        LOGE() << errorString();
    }

#ifndef NDEBUG
    auto end = std::chrono::steady_clock::now();
    acc.total_ms += std::chrono::duration<double, std::milli>(end - start).count();
    acc.count++;
#endif //NDEBUG
}

XmlDomElement XmlDomDocument::rootElement() const
{
    xml_element_impl e = m_xml->doc.FirstChildElement();
    return XmlDomElement(m_xml, pack_handle(e));
}

bool XmlDomDocument::hasError() const
{
    return m_xml->err != tinyxml2::XML_SUCCESS;
}

String XmlDomDocument::errorString() const
{
    return String::fromUtf8(m_xml->doc.ErrorStr());
}
