//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

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
    XmlWriter(IODevice* device);
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
    void initWriter(IODevice* device);

    std::unique_ptr<IODevice> m_device;
    std::unique_ptr<QXmlStreamWriter> m_writer;
};
}

#endif // MU_FRAMEWORK_XMLWRITER_H
