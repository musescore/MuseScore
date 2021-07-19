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
#ifndef MU_DRAW_BRUSH_H
#define MU_DRAW_BRUSH_H

#include <QColor>
#include "drawtypes.h"

namespace mu::draw {
class Brush
{
public:
    Brush() {}
    Brush(BrushStyle style)
        : m_style(style) {}
    Brush(const QColor& color)
        : m_color(color) {}
    Brush(Qt::GlobalColor color)
        : m_color(color) {}

    inline BrushStyle style() const { return m_style; }
    void setStyle(BrushStyle style) { m_style = style; }
    inline const QColor& color() const { return m_color; }
    void setColor(const QColor& color) { m_color = color; }
    inline void setColor(Qt::GlobalColor color) { m_color = color; }

#ifndef NO_QT_SUPPORT
    static QBrush toQBrush(const Brush& brush)
    {
        return QBrush(brush.m_color, static_cast<Qt::BrushStyle>(brush.m_style));
    }

    static Brush fromQBrush(const QBrush& qbrush)
    {
        Brush brush(qbrush.color());
        brush.setStyle(static_cast<BrushStyle>(qbrush.style()));
        return brush;
    }

#endif

private:
    QColor m_color = Qt::black;
    BrushStyle m_style = BrushStyle::SolidPattern;
};
} // namespace mu::draw

#endif // MU_DRAW_BRUSH_H
