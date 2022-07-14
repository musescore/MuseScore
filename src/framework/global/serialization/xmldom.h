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
#ifndef MU_GLOBAL_XMLDOM_H
#define MU_GLOBAL_XMLDOM_H

#include <memory>

#include <QByteArray>

#include "types/string.h"

namespace mu {
struct XmlDomData;
struct XmlNodeData;

class XmlDomElement;
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

    bool hasAttribute(const char* name) const;
    String attribute(const char* name) const;

    XmlDomElement toElement() const;

protected:
    friend class XmlDomDocument;
    friend class XmlDomElement;

    XmlDomNode(const std::shared_ptr<XmlDomData>& data, const std::shared_ptr<XmlNodeData>& node);

    std::shared_ptr<XmlDomData> m_data = nullptr;
    std::shared_ptr<XmlNodeData> m_node = nullptr;
};

class XmlDomElement : public XmlDomNode
{
public:

    String text() const;

private:

    friend class XmlDomDocument;
    friend class XmlDomNode;

    XmlDomElement(const std::shared_ptr<XmlDomData>& data, const std::shared_ptr<XmlNodeData>& node);
};

class XmlDomDocument
{
public:
    XmlDomDocument();

    void setContent(const QByteArray& data);

    XmlDomElement documentElement() const;

private:
    std::shared_ptr<XmlDomData> m_data = nullptr;
};
}

#endif // MU_GLOBAL_XMLDOM_H
