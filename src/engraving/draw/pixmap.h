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

#include <QByteArray>

#include "geometry.h"

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
    QByteArray data() const { return m_data; }

    bool isNull() const { return m_data.isNull(); }

    void setData(const QByteArray& data)
    {
        m_data = data;
        m_key = qHash(data);
    }

    uint key() const { return m_key; }

private:
    Size m_size;
    QByteArray m_data; //! usually png
    uint m_key;
};
}

#endif // MU_DRAW_PIXMAP_H
