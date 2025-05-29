/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#pragma once

#include "types/bytearray.h"
#include "serialization/xmldom.h"
#include "musx/musx.h"

namespace mu::iex::finale::xml {

class Attribute final : public musx::factory::IXmlAttribute
{
    muse::XmlDomAttribute attr;

public:
    explicit Attribute(muse::XmlDomAttribute a) : attr(a) {}

    std::string getName() const override {
        return attr.attributeName().toStdString();
    }

    std::string getValue() const override {
        return attr.value().toStdString();
    }

    std::shared_ptr<IXmlAttribute> nextAttribute() const override {
        auto next = attr.nextAttribute();
        return next.isNull() ? nullptr : std::make_shared<Attribute>(next);
    }
};

class Element final : public musx::factory::IXmlElement
{
    muse::XmlDomElement elem;

    static const char* tagPtr(const std::string& tag) {
        return tag.empty() ? nullptr : tag.c_str();
    }

public:
    explicit Element(muse::XmlDomElement e) : elem(e) {}

    std::string getTagName() const override {
        return elem.nodeName().toStdString();
    }

    std::string getText() const override {
        return elem.text().toStdString();
    }

    std::shared_ptr<Attribute::IXmlAttribute> getFirstAttribute() const override {
        auto attr = elem.firstAttribute();
        return attr.isNull() ? nullptr : std::make_shared<Attribute>(attr);
    }

    std::shared_ptr<Attribute::IXmlAttribute> findAttribute(const std::string& name) const override {
        auto attr = elem.attribute(name.c_str());
        return attr.isNull() ? nullptr : std::make_shared<Attribute>(attr);
    }

    musx::factory::XmlElementPtr getFirstChildElement(const std::string& tag = {}) const override {
        auto child = elem.firstChildElement(tagPtr(tag));
        return child.isNull() ? nullptr : std::make_shared<Element>(child);
    }

    musx::factory::XmlElementPtr getNextSibling(const std::string& tag = {}) const override {
        auto sib = elem.nextSiblingElement(tagPtr(tag));
        return sib.isNull() ? nullptr : std::make_shared<Element>(sib);
    }

    musx::factory::XmlElementPtr getPreviousSibling(const std::string& tag = {}) const override {
        auto sib = elem.previousSiblingElement(tagPtr(tag));
        return sib.isNull() ? nullptr : std::make_shared<Element>(sib);
    }

    musx::factory::XmlElementPtr getParent() const override {
        auto parent = elem.parent().toElement();
        return parent.isNull() ? nullptr : std::make_shared<Element>(parent);
    }
};

class Document final : public musx::factory::IXmlDocument
{
    muse::XmlDomDocument doc;

public:
    void loadFromBuffer(const char* data, size_t size) override {
        muse::ByteArray bytes = muse::ByteArray::fromRawData(data, size);
        doc.setContent(bytes);
        if (doc.hasError()) {
            throw musx::factory::load_error(doc.errorString().toStdString());
        }
    }

    void loadFromString(const std::string& xmlContent) override {
        loadFromBuffer(xmlContent.data(), xmlContent.size());
    }

    std::shared_ptr<Element::IXmlElement> getRootElement() const override {
        auto root = doc.rootElement();
        return root.isNull() ? nullptr : std::make_shared<Element>(root);
    }
};

}
