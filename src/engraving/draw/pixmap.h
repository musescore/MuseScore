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

#include "modularity/ioc.h"

#include "geometry.h"

namespace mu::draw {
class Pixmap
{
public:

    Pixmap() = default;
    Pixmap(Size size)
        : m_Size(size) {}

    int width() const { return m_Size.width(); }
    int height() const { return m_Size.height(); }
    Size size() const { return m_Size; }
    QByteArray data() const { return m_Data; }

    bool isNull() const { return m_Data.isNull(); }

    void setData(const QByteArray& data)
    {
        m_Data = data;
        m_cashKey = qHash(data);
    }

    uint cashKey() const { return m_cashKey; }

private:
    Size m_Size;
    QByteArray m_Data;
    uint m_cashKey;
};
}

#endif // MU_DRAW_PIXMAP_H
