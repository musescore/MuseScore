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

#ifndef MU_FRAMEWORK_XMLWRITER_H
#define MU_FRAMEWORK_XMLWRITER_H

#include <memory>

#include "io/path.h"
#include "system/iodevice.h"

class QXmlStreamWriter;

namespace mu::framework {
class XmlWriter
{
public:
    XmlWriter(const io::path& path);
    XmlWriter(system::IODevice* device);
    ~XmlWriter();

    void writeStartDocument(std::string_view version = std::string_view());
    void writeEndDocument();

    void writeStartElement(std::string_view name);
    void writeEndElement();

    void writeAttribute(std::string_view name, const std::string& value);
    void writeCharacters(const std::string& text);
    void writeTextElement(std::string_view name, const std::string& text);

    bool success() const;

private:
    void initWriter(system::IODevice* device);

    std::unique_ptr<system::IODevice> m_device;
    std::unique_ptr<QXmlStreamWriter> m_writer;
};
}

#endif // MU_FRAMEWORK_XMLWRITER_H
