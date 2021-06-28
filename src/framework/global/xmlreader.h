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

#ifndef MU_FRAMEWORK_XMLREADER_H
#define MU_FRAMEWORK_XMLREADER_H

#include <memory>

#include "io/path.h"
#include "io/device.h"

class QXmlStreamReader;
class QByteArray;

namespace mu::framework {
class XmlReader
{
public:
    XmlReader(const io::path& path);
    XmlReader(io::Device* device);
    XmlReader(const QByteArray& bytes);
    ~XmlReader();

    bool readNextStartElement();
    bool canRead() const;
    void skipCurrentElement();
    std::string tagName() const;

    enum TokenType {
        Unknown,
        StartDocument,
        EndDocument,
        StartElement,
        EndElement,
        Characters,
        Comment
    };

    TokenType readNext();
    TokenType tokenType() const;

    int intAttribute(std::string_view name, int defaultValue = 0) const;
    double doubleAttribute(std::string_view name, double defaultValue = 0.) const;
    std::string attribute(std::string_view name) const;
    bool hasAttribute(std::string_view name) const;

    enum ReadStringBehavior {
        ErrorOnUnexpectedElement,
        IncludeChildElements,
        SkipChildElements
    };

    std::string readString(ReadStringBehavior behavior = ErrorOnUnexpectedElement);
    int readInt();
    double readDouble();

    bool success() const;
    std::string error() const;

private:
    QString readElementText(ReadStringBehavior behavior = ErrorOnUnexpectedElement);
    QStringRef attributeValue(std::string_view name) const;

    std::unique_ptr<io::Device> m_device;
    std::unique_ptr<QXmlStreamReader> m_reader;
};
}

#endif // MU_FRAMEWORK_XMLREADER_H
