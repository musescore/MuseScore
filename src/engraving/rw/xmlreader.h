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
#ifndef MU_ENGRAVING_XMLREADER_H
#define MU_ENGRAVING_XMLREADER_H

#include <map>
#include <QXmlStreamReader>
#include <QTextStream>
#include <QFile>

#include <unordered_map>

#include "containers.h"

#include "infrastructure/draw/color.h"
#include "libmscore/connector.h"
#include "libmscore/stafftype.h"
#include "libmscore/interval.h"
#include "libmscore/engravingitem.h"
#include "libmscore/select.h"

namespace mu::engraving {
class ReadContext;
}

namespace Ms {
class XmlReader : public QXmlStreamReader
{
    QString docName;    // used for error reporting

    qint64 _offsetLines = 0;

    void htmlToString(int level, QString*);

    mutable mu::engraving::ReadContext* m_context = nullptr;
    mutable bool m_selfContext = false;

public:
    XmlReader(QFile* f)
        : QXmlStreamReader(f), docName(f->fileName()) {}
    XmlReader(const QByteArray& d)
        : QXmlStreamReader(d) {}
    XmlReader(QIODevice* d)
        : QXmlStreamReader(d) {}
    XmlReader(const QString& d)
        : QXmlStreamReader(d) {}

    XmlReader(const XmlReader&) = delete;
    XmlReader& operator=(const XmlReader&) = delete;

    ~XmlReader();

    void unknown();

    // attribute helper routines:
    QString attribute(const char* s) const { return attributes().value(s).toString(); }
    QString attribute(const char* s, const QString&) const;
    int intAttribute(const char* s) const;
    int intAttribute(const char* s, int _default) const;
    double doubleAttribute(const char* s) const;
    double doubleAttribute(const char* s, double _default) const;
    bool hasAttribute(const char* s) const;

    // helper routines based on readElementText():
    int readInt() { return readElementText().toInt(); }
    int readInt(bool* ok) { return readElementText().toInt(ok); }
    int readIntHex() { return readElementText().toInt(0, 16); }
    double readDouble() { return readElementText().toDouble(); }
    qlonglong readLongLong() { return readElementText().toLongLong(); }

    double readDouble(double min, double max);
    bool readBool();
    mu::PointF readPoint();
    mu::SizeF readSize();
    mu::ScaleF readScale();
    mu::RectF readRect();
    mu::draw::Color readColor();
    Fraction readFraction();
    QString readXml();

    void setDocName(const QString& s) { docName = s; }
    QString getDocName() const { return docName; }

    // for reading old files (< 3.01)
    void setOffsetLines(qint64 val) { _offsetLines = val; }

    mu::engraving::ReadContext* context() const;
    void setContext(mu::engraving::ReadContext* context);
};
}

#endif // MU_ENGRAVING_XMLREADER_H
