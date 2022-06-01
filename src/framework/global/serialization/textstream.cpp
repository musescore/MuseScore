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
#include "textstream.h"

using namespace mu;

static constexpr int TEXTSTREAM_BUFFERSIZE = 16384;

TextStream::TextStream(io::IODevice* device)
    : m_device(device), m_ref(&m_buf)
{
}

TextStream::TextStream(QString* str)
    : m_ref(str)
{
}

TextStream::~TextStream()
{
    flush();
}

void TextStream::setDevice(io::IODevice* device)
{
    m_device = device;
    m_ref = &m_buf;
}

void TextStream::setString(QString* str)
{
    m_device = nullptr;
    m_ref = str;
}

void TextStream::flush()
{
    if (m_device && m_device->isOpen()) {
        QByteArray data = m_ref->toUtf8();
        ByteArray ba = ByteArray::fromQByteArrayNoCopy(data);
        m_device->write(ba);
        m_ref->clear();
    }
}

TextStream& TextStream::operator<<(char ch)
{
    QChar c = QChar::fromLatin1(ch);
    write(&c, 1);
    return *this;
}

TextStream& TextStream::operator<<(int val)
{
    QString s = QString::number(val);
    write(s.constData(), s.length());
    return *this;
}

TextStream& TextStream::operator<<(double val)
{
    QString s = QString::number(val);
    write(s.constData(), s.length());
    return *this;
}

TextStream& TextStream::operator<<(int64_t val)
{
    QString s = QString::number(val);
    write(s.constData(), s.length());
    return *this;
}

TextStream& TextStream::operator<<(const char* s)
{
    QString str(s);
    write(str.constData(), str.length());
    return *this;
}

TextStream& TextStream::operator<<(const QString& s)
{
    write(s.constData(), s.length());
    return *this;
}

TextStream& TextStream::operator<<(const ByteArray& b)
{
    QString s = QString::fromUtf8(reinterpret_cast<const char*>(b.constData()), static_cast<int>(b.size()));
    write(s.constData(), s.length());
    return *this;
}

TextStream& TextStream::operator<<(const AsciiString& s)
{
    QString str(s.ascii());
    write(str.constData(), str.length());
    return *this;
}

TextStream& TextStream::operator<<(const String& s)
{
    QString qs = s.toQString();
    write(qs.constData(), qs.length());
    return *this;
}

void TextStream::write(const QChar* ch, int len)
{
    if (m_ref) {
        m_ref->append(ch, len);
    }

    if (m_device && m_ref->size() > TEXTSTREAM_BUFFERSIZE) {
        flush();
    }
}
