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

// ================================================
// XmlDomNode
// ================================================

static inline const tinyxml2::XMLNode* node_const(xml_node_ptr p)
{
    return reinterpret_cast<const tinyxml2::XMLNode*>(p);
}

XmlDomNode::XmlDomNode(const std::shared_ptr<XmlDomImplData>& xml, xml_node_ptr node)
    : m_xml(xml), m_node(node)
{
}

bool XmlDomNode::isNull() const
{
    return m_node ? false : true;
}

String XmlDomNode::nodeName() const
{
    return m_node ? String::fromUtf8(node_const(m_node)->Value()) : String();
}

bool XmlDomNode::hasChildNodes() const
{
    return m_node ? !node_const(m_node)->NoChildren() : false;
}

XmlDomNode XmlDomNode::firstChild() const
{
    if (!m_node) {
        return XmlDomNode(m_xml, 0);
    }
    const tinyxml2::XMLNode* n = node_const(m_node)->FirstChild();
    return XmlDomNode(m_xml, reinterpret_cast<xml_node_ptr>(n));
}

XmlDomElement XmlDomNode::firstChildElement(const char* name) const
{
    if (!m_node) {
        return XmlDomElement(m_xml, 0);
    }

    const tinyxml2::XMLElement* n = node_const(m_node)->FirstChildElement(name);
    return XmlDomElement(m_xml, reinterpret_cast<xml_node_ptr>(n));
}

XmlDomNode XmlDomNode::nextSibling() const
{
    if (!m_node) {
        return XmlDomNode(m_xml, 0);
    }
    const tinyxml2::XMLNode* n = node_const(m_node)->NextSibling();
    return XmlDomNode(m_xml, reinterpret_cast<xml_node_ptr>(n));
}

XmlDomNode XmlDomNode::previousSibling() const
{
    if (!m_node) {
        return XmlDomNode(m_xml, 0);
    }
    const tinyxml2::XMLNode* n = node_const(m_node)->PreviousSibling();
    return XmlDomNode(m_xml, reinterpret_cast<xml_node_ptr>(n));
}

XmlDomNode XmlDomNode::parent() const
{
    if (!m_node) {
        return XmlDomNode(m_xml, 0);
    }
    const tinyxml2::XMLNode* n = node_const(m_node)->Parent();
    return XmlDomNode(m_xml, reinterpret_cast<xml_node_ptr>(n));
}

XmlDomElement XmlDomNode::nextSiblingElement(const char* name) const
{
    if (!m_node) {
        return XmlDomElement(m_xml, 0);
    }
    const tinyxml2::XMLElement* e = node_const(m_node)->NextSiblingElement(name);
    return XmlDomElement(m_xml, reinterpret_cast<xml_node_ptr>(e));
}

XmlDomElement XmlDomNode::previousSiblingElement(const char* name) const
{
    if (!m_node) {
        return XmlDomElement(m_xml, 0);
    }
    const tinyxml2::XMLElement* e = node_const(m_node)->PreviousSiblingElement(name);
    return XmlDomElement(m_xml, reinterpret_cast<xml_node_ptr>(e));
}

XmlDomElement XmlDomNode::toElement() const
{
    const tinyxml2::XMLElement* e = node_const(m_node)->ToElement();
    return XmlDomElement(m_xml, reinterpret_cast<xml_node_ptr>(e));
}

// ================================================
// XmlDomAttribute
// ================================================

static inline const tinyxml2::XMLAttribute* attribute_const(xml_attribute_ptr p)
{
    return reinterpret_cast<const tinyxml2::XMLAttribute*>(p);
}

bool XmlDomAttribute::isNull() const
{
    return m_attribute ? false : true;
}

String XmlDomAttribute::attributeName() const
{
    return m_attribute ? String::fromUtf8(attribute_const(m_attribute)->Name()) : String();
}

String XmlDomAttribute::value() const
{
    if (!m_attribute) {
        return String();
    }
    const tinyxml2::XMLAttribute* a = attribute_const(m_attribute);
    return String::fromUtf8(a->Value());
}

XmlDomAttribute XmlDomAttribute::nextAttribute() const
{
    if (!m_attribute) {
        return XmlDomAttribute(0);
    }
    const tinyxml2::XMLAttribute* a = attribute_const(m_attribute);
    return XmlDomAttribute(reinterpret_cast<xml_attribute_ptr>(a->Next()));
}

// ================================================
// XmlDomElement
// ================================================

static inline const tinyxml2::XMLElement* el_const(xml_node_ptr p)
{
    return reinterpret_cast<const tinyxml2::XMLNode*>(p)->ToElement();
}

XmlDomElement::XmlDomElement(const std::shared_ptr<XmlDomImplData>& data, xml_node_ptr node)
    : XmlDomNode(data, node)
{
}

String XmlDomElement::text() const
{
    const tinyxml2::XMLElement* e = el_const(m_node);
    if (!e) {
        return String();
    }

    String result;
    for (const tinyxml2::XMLNode* n = e->FirstChild(); n != nullptr; n = n->NextSibling()) {
        const tinyxml2::XMLText* t = n->ToText();
        if (t) {
            result += String::fromUtf8(t->Value());
        }
    }

    return result;
}

XmlDomAttribute XmlDomElement::firstAttribute() const
{
    if (!m_node) {
        return XmlDomAttribute(0);
    }
    if (const tinyxml2::XMLElement* e = el_const(m_node)) {
        return XmlDomAttribute(reinterpret_cast<xml_attribute_ptr>(e->FirstAttribute()));
    }
    return XmlDomAttribute(0);
}

XmlDomAttribute XmlDomElement::attribute(const char* name) const
{
    if (!m_node) {
        return XmlDomAttribute(0);
    }
    if (const tinyxml2::XMLElement* e = el_const(m_node)) {
        return XmlDomAttribute(reinterpret_cast<xml_attribute_ptr>(e->FindAttribute(name)));
    }
    return XmlDomAttribute(0);
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
    m_xml->doc.Clear();
    m_xml->err = m_xml->doc.Parse(reinterpret_cast<const char*>(data.constData()), data.size());

    if (m_xml->err != tinyxml2::XML_SUCCESS) {
        LOGE() << errorString();
    }
}

XmlDomElement XmlDomDocument::rootElement() const
{
    const tinyxml2::XMLElement* e = m_xml->doc.FirstChildElement();
    return XmlDomElement(m_xml, reinterpret_cast<xml_node_ptr>(e));
}

bool XmlDomDocument::hasError() const
{
    return m_xml->err != tinyxml2::XML_SUCCESS;
}

String XmlDomDocument::errorString() const
{
    return String::fromUtf8(m_xml->doc.ErrorStr());
}
