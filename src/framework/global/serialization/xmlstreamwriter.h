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
#ifndef MU_GLOBAL_XMLSTREAMWRITER_H
#define MU_GLOBAL_XMLSTREAMWRITER_H

#include <list>
#include <QIODevice>
#include <QString>

class QTextStream;

namespace mu {
class XmlStreamWriter
{
public:
    XmlStreamWriter();
    explicit XmlStreamWriter(QIODevice* dev);
    virtual ~XmlStreamWriter();

    void setDevice(QIODevice* dev);
    void setString(QString* string, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    void flush();

    void writeHeader();
    void writeDoctype(const QString& type);

    void writeStartElement(const QString& name);
    void writeStartElement(const QString& name, const QString& attributes);
    void writeEndElement();

    void writeElement(const QString& name, const QString& val);
    void writeElement(const QString& name, int val);
    void writeElement(const QString& name, qint64 val);
    void writeElement(const QString& name, double val);

    void writeElement(const QString& nameWithAttributes);

    void writeComment(const QString& text);

private:

    void putLevel();

    std::list<QString> m_stack;

    //! NOTE Temporary implementation
    QTextStream* m_stream = nullptr;
};
}

#endif // MU_GLOBAL_XMLSTREAMWRITER_H
