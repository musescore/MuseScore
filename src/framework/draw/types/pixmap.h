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
#ifndef MU_DRAW_PIXMAP_H
#define MU_DRAW_PIXMAP_H

#include "types/bytearray.h"
#include "geometry.h"

#ifndef NO_QT_SUPPORT
#include <QPixmap>
#include <QBuffer>
#endif

namespace mu::draw {
class Pixmap
{
public:

    Pixmap() = default;
    Pixmap(Size size)
        : m_size(size) {}

    int width() const { return m_size.width(); }
    int height() const { return m_size.height(); }
    Size size() const { return m_size; }
    ByteArray data() const { return m_data; }

    bool isNull() const { return m_data.empty(); }

    unsigned int key() const { return m_key; }

#ifndef NO_QT_SUPPORT
    static Pixmap fromQPixmap(const QPixmap& qtPixmap)
    {
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        qtPixmap.save(&buffer, "PNG");

        Pixmap result({ qtPixmap.width(), qtPixmap.height() });
        result.setData(ByteArray::fromQByteArray(bytes));

        return result;
    }

    static QPixmap toQPixmap(const Pixmap& pixmap)
    {
        QPixmap qtPixMap;
        qtPixMap.loadFromData(pixmap.data().toQByteArrayNoCopy());

        return qtPixMap;
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
Q_DECLARE_METATYPE(mu::draw::Pixmap)
#endif

#endif // MU_DRAW_PIXMAP_H
