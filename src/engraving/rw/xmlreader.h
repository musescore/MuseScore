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
#ifndef MU_ENGRAVING_XMLREADER_H
#define MU_ENGRAVING_XMLREADER_H

#include <map>

#include "serialization/xmlstreamreader.h"

#include "draw/types/color.h"
#include "draw/types/geometry.h"

#include "../types/types.h"

namespace mu::engraving {
class XmlReader : public muse::XmlStreamReader
{
public:

    XmlReader() = default;
    XmlReader(const muse::ByteArray& d)
        : XmlStreamReader(d) {}
    XmlReader(muse::io::IODevice* d)
        : XmlStreamReader(d) {}

#ifndef NO_QT_SUPPORT
    XmlReader(const QByteArray& d)
        : XmlStreamReader(d) {}
#endif

    XmlReader(const XmlReader&) = delete;
    XmlReader& operator=(const XmlReader&) = delete;

    ~XmlReader();

    void unknown();

    bool readBool() { return XmlStreamReader::readInt(); }
    double readDouble(bool* ok = nullptr) { return XmlStreamReader::readDouble(ok); }
    double readDouble(double min, double max);

    PointF readPoint();
    SizeF readSize();
    ScaleF readScale();
    RectF readRect();
    Color readColor();
    Fraction readFraction();
    String readXml();

    void setDocName(const String& s) { m_docName = s; }
    String docName() const { return m_docName; }

    // for reading old files (< 3.01)
    void setOffsetLines(int64_t val) { m_offsetLines = val; }

private:

    void htmlToString(int level, String*);

    String m_docName;    // used for error reporting
    int64_t m_offsetLines = 0;
};
}

#endif // MU_ENGRAVING_XMLREADER_H
