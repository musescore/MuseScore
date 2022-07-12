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

#include <QDomDocument>

using namespace mu;

struct mu::XmlDomData
{
    QDomDocument doc;
};

struct mu::XmlNodeData
{
    QDomNode node;
    XmlNodeData(const QDomNode& n)
        : node(n) {}
};

// ================================================
// XmlDomNode
// ================================================

XmlDomNode::XmlDomNode(const std::shared_ptr<XmlDomData>& data, const std::shared_ptr<XmlNodeData>& node)
    : m_data(data), m_node(node)
{
}

bool XmlDomNode::isNull() const
{
    return m_node ? m_node->node.isNull() : true;
}

String XmlDomNode::nodeName() const
{
    return m_node->node.nodeName();
}

bool XmlDomNode::hasChildNodes() const
{
    return m_node->node.hasChildNodes();
}

XmlDomNode XmlDomNode::firstChild() const
{
    QDomNode n = m_node->node.firstChild();
    return XmlDomNode(m_data, std::make_shared<XmlNodeData>(n));
}

XmlDomElement XmlDomNode::firstChildElement(const char* name) const
{
    QDomNode n = m_node->node.firstChildElement(name);
    return XmlDomElement(m_data, std::make_shared<XmlNodeData>(n));
}

XmlDomNode XmlDomNode::nextSibling() const
{
    QDomNode n = m_node->node.nextSibling();
    return XmlDomNode(m_data, std::make_shared<XmlNodeData>(n));
}

bool XmlDomNode::hasAttribute(const char* name) const
{
    return !m_node->node.attributes().namedItem(name).isNull();
}

String XmlDomNode::attribute(const char* name) const
{
    return m_node->node.attributes().namedItem(name).toAttr().value();
}

XmlDomElement XmlDomNode::toElement() const
{
    return XmlDomElement(m_data, m_node);
}

// ================================================
// XmlDomElement
// ================================================

static QDomElement to_el(const std::shared_ptr<XmlNodeData>& node)
{
    return node->node.toElement();
}

XmlDomElement::XmlDomElement(const std::shared_ptr<XmlDomData>& data, const std::shared_ptr<XmlNodeData>& node)
    : XmlDomNode(data, node)
{
}

String XmlDomElement::text() const
{
    return to_el(m_node).text();
}

// ================================================
// XmlDomDoсument
// ================================================

XmlDomDocument::XmlDomDocument()
{
    m_data = std::make_shared<XmlDomData>();
}

void XmlDomDocument::setContent(const QByteArray& data)
{
    m_data->doc.setContent(data);
}

XmlDomElement XmlDomDocument::documentElement() const
{
    QDomElement el = m_data->doc.documentElement();
    return XmlDomElement(m_data, std::make_shared<XmlNodeData>(el));
}
