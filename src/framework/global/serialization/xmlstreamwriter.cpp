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
#include "xmlstreamwriter.h"

#include <QTextStream>

#include "containers.h"

using namespace mu;

XmlStreamWriter::XmlStreamWriter()
{
    m_stream = new QTextStream();
    m_stream->setCodec("UTF-8");
}

XmlStreamWriter::XmlStreamWriter(QIODevice* dev)
{
    m_stream = new QTextStream(dev);
    m_stream->setCodec("UTF-8");
}

XmlStreamWriter::~XmlStreamWriter()
{
    delete m_stream;
}

void XmlStreamWriter::setDevice(QIODevice* dev)
{
    m_stream->setDevice(dev);
}

void XmlStreamWriter::setString(QString* string, QIODevice::OpenMode openMode)
{
    m_stream->setString(string, openMode);
}

void XmlStreamWriter::flush()
{
    m_stream->flush();
}

void XmlStreamWriter::putLevel()
{
    size_t level = m_stack.size();
    for (size_t i = 0; i < level * 2; ++i) {
        *m_stream << ' ';
    }
}

void XmlStreamWriter::writeHeader()
{
    *m_stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
}

void XmlStreamWriter::writeDoctype(const QString& type)
{
    *m_stream << "<!DOCTYPE " << type << ">\n";
}

void XmlStreamWriter::writeStartElement(const QString& name)
{
    putLevel();
    *m_stream << '<' << name << '>' << Qt::endl;
    m_stack.push_back(name.split(' ')[0]);
}

void XmlStreamWriter::writeStartElement(const QString& name, const QString& attributes)
{
    putLevel();
    *m_stream << '<' << name;
    if (!attributes.isEmpty()) {
        *m_stream << ' ' << attributes;
    }
    *m_stream << '>' << Qt::endl;
    m_stack.push_back(name);
}

void XmlStreamWriter::writeEndElement()
{
    putLevel();
    *m_stream << "</" << mu::takeLast(m_stack) << '>' << Qt::endl;
}

void XmlStreamWriter::writeElement(const QString& name, const QString& val)
{
    QString ename(QString(name).split(' ')[0]);
    putLevel();
    *m_stream << "<" << name << ">";
    *m_stream << val;
    *m_stream << "</" << ename << ">\n";
}

void XmlStreamWriter::writeElement(const QString& name, int val)
{
    QString ename(QString(name).split(' ')[0]);
    putLevel();
    *m_stream << "<" << name << ">";
    *m_stream << val;
    *m_stream << "</" << ename << ">\n";
}

void XmlStreamWriter::writeElement(const QString& name, qint64 val)
{
    QString ename(QString(name).split(' ')[0]);
    putLevel();
    *m_stream << "<" << name << ">";
    *m_stream << val;
    *m_stream << "</" << ename << ">\n";
}

void XmlStreamWriter::writeElement(const QString& name, double val)
{
    QString ename(QString(name).split(' ')[0]);
    putLevel();
    *m_stream << "<" << name << ">";
    *m_stream << val;
    *m_stream << "</" << ename << ">\n";
}

void XmlStreamWriter::writeElement(const QString& nameWithAttributes)
{
    putLevel();
    *m_stream << '<' << nameWithAttributes << "/>\n";
}

void XmlStreamWriter::writeComment(const QString& text)
{
    putLevel();
    *m_stream << "<!-- " << text << " -->\n";
}
