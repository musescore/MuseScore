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
#ifndef MUSE_DRAW_PIXMAP_H
#define MUSE_DRAW_PIXMAP_H

#include "global/types/bytearray.h"

#include "geometry.h"

#ifndef NO_QT_SUPPORT
#include <QPixmap>
#include <QImage>
#include <QBuffer>
#endif

namespace muse::draw {
class Pixmap
{
public:

    Pixmap() = default;
    Pixmap(Size size)
        : m_size(size) {}
    Pixmap(int w, int h)
        : m_size(Size(w, h)) {}

    int width() const { return m_size.width(); }
    int height() const { return m_size.height(); }
    Size size() const { return m_size; }
    ByteArray data() const { return m_data; }

    bool isNull() const { return m_data.empty(); }

    unsigned int key() const { return m_key; }

    bool operator==(const Pixmap& o) const { return m_size == o.m_size && m_key == o.m_key && m_data == o.m_data; }
    bool operator!=(const Pixmap& o) const { return !this->operator==(o); }

#ifndef NO_QT_SUPPORT
    static Pixmap fromQImage(const QImage& qim)
    {
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        qim.save(&buffer, "PNG");

        Pixmap px(qim.width(), qim.height());
        px.setData(ByteArray::fromQByteArray(bytes));

        return px;
    }

    static QImage toQImage(const Pixmap& px)
    {
        QImage qim(px.width(), px.height(), QImage::Format_ARGB32_Premultiplied);
        ByteArray data = px.data();
        if (!data.empty()) {
            qim.loadFromData(data.toQByteArrayNoCopy());
        }
        return qim;
    }

    static Pixmap fromQPixmap(const QPixmap& qpx)
    {
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        qpx.save(&buffer, "PNG");

        Pixmap px(qpx.width(), qpx.height());
        px.setData(ByteArray::fromQByteArray(bytes));

        return px;
    }

    static QPixmap toQPixmap(const Pixmap& px)
    {
        QPixmap qpx(px.width(), px.height());
        ByteArray data = px.data();
        if (!data.empty()) {
            qpx.loadFromData(data.toQByteArrayNoCopy());
        }
        return qpx;
    }

#endif

private:

    inline unsigned int doKey(const uint8_t* p, size_t len) const
    {
        unsigned int h = 0;
        for (size_t i = 0; i < len; ++i) {
            h = 31 * h + p[i];
        }
        return h;
    }

    void setData(const ByteArray& data)
    {
        m_data = data;
        m_key = doKey(data.constData(), data.size());
    }

    Size m_size;
    ByteArray m_data; //! usually png
    unsigned int m_key = 0;
};
}

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(muse::draw::Pixmap)
#endif

#endif // MUSE_DRAW_PIXMAP_H
