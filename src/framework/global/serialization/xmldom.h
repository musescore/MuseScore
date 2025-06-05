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
#ifndef MUSE_GLOBAL_XMLDOM_H
#define MUSE_GLOBAL_XMLDOM_H

#include <memory>

#include "types/bytearray.h"
#include "types/string.h"

namespace muse {
struct XmlDomImplData;
// avoid external dependency on a particular xml library
using xml_node_ptr = uintptr_t;
using xml_attribute_ptr = uintptr_t;

class XmlDomElement;
class XmlDomAttribute;
class XmlDomNode
{
public:

    XmlDomNode() = default;

    bool isNull() const;
    String nodeName() const;

    bool hasChildNodes() const;
    XmlDomNode firstChild() const;
    XmlDomElement firstChildElement(const char* name) const;
    XmlDomNode nextSibling() const;
    XmlDomNode previousSibling() const;
    XmlDomNode parent() const;

    XmlDomElement nextSiblingElement(const char* name = nullptr) const;
    XmlDomElement previousSiblingElement(const char* name = nullptr) const;
    XmlDomElement toElement() const;

protected:
    friend class XmlDomDocument;
    friend class XmlDomElement;

    XmlDomNode(const std::shared_ptr<XmlDomImplData>& xml, xml_node_ptr node);

    std::shared_ptr<XmlDomImplData> m_xml = nullptr;
    xml_node_ptr m_node = 0;
};

class XmlDomElement : public XmlDomNode
{
public:

    String text() const;

    XmlDomAttribute firstAttribute() const;
    XmlDomAttribute attribute(const char* name) const;

private:

    friend class XmlDomDocument;
    friend class XmlDomNode;

    XmlDomElement(const std::shared_ptr<XmlDomImplData>& data, xml_node_ptr node);
};

class XmlDomAttribute
{
public:

    bool isNull() const;
    String attributeName() const;

    String value() const;

    XmlDomAttribute nextAttribute() const;

private:
    friend class XmlDomElement;

    explicit XmlDomAttribute(xml_attribute_ptr attribute)
        : m_attribute(attribute) {}

    xml_attribute_ptr m_attribute = 0;
};

class XmlDomDocument
{
public:
    XmlDomDocument();

    void setContent(const ByteArray& data);

    XmlDomElement rootElement() const;

    bool hasError() const;
    String errorString() const;

private:
    std::shared_ptr<XmlDomImplData> m_xml = nullptr;
};
}

#endif // MUSE_GLOBAL_XMLDOM_H
