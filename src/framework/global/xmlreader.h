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

#ifndef MU_FRAMEWORK_XMLREADER_H
#define MU_FRAMEWORK_XMLREADER_H

#include <memory>

#include "io/path.h"
#include "system/iodevice.h"

class QXmlStreamReader;
class QByteArray;

namespace mu::framework {
class XmlReader
{
public:
    XmlReader(const io::path& path);
    XmlReader(IODevice* device);
    XmlReader(const QByteArray& bytes);
    ~XmlReader();

    bool readNextStartElement();
    void skipCurrentElement();
    std::string tagName() const;

    int intAttribute(std::string_view name, int defaultValue = 0) const;
    double doubleAttribute(std::string_view name, double defaultValue = 0.) const;
    std::string attribute(std::string_view name) const;
    bool hasAttribute(std::string_view name) const;

    int readInt();
    double readDouble();
    std::string readString();

private:
    QString readElementText();
    QStringRef attributeValue(std::string_view name) const;

    std::unique_ptr<IODevice> m_device;
    std::unique_ptr<QXmlStreamReader> m_reader;
};
}

#endif // MU_FRAMEWORK_XMLREADER_H
