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
#ifndef MUSE_DRAW_PEN_H
#define MUSE_DRAW_PEN_H

#ifndef NO_QT_SUPPORT
#include <QPen>
#endif

#include "global/realfn.h"

#include "color.h"
#include "drawtypes.h"

namespace muse::draw {
class Pen
{
public:

    Pen(PenStyle style)
        : m_style(style)
    {
    }

    Pen(const Color& color = Color::BLACK, double width = 1, PenStyle s = PenStyle::SolidLine,
        PenCapStyle c = PenCapStyle::SquareCap, PenJoinStyle j = PenJoinStyle::BevelJoin)
        : m_color(color), m_width(width), m_style(s), m_capStyle(c), m_joinStyle(j)
    {
    }

    inline bool operator==(const Pen& o) const
    {
        return m_color == o.m_color
               && RealIsEqual(m_width, o.m_width)
               && m_style == o.m_style
               && m_capStyle == o.m_capStyle
               && m_joinStyle == o.m_joinStyle
               && RealIsEqual(m_dashPattern, o.m_dashPattern);
    }

    inline bool operator!=(const Pen& o) const { return !this->operator==(o); }

    PenStyle style() const
    {
        return m_style;
    }

    void setStyle(PenStyle style)
    {
        if (m_style == style) {
            return;
        }
        m_style = style;
        m_dashPattern.clear();
    }

    std::vector<double> dashPattern() const
    {
        if (m_style == PenStyle::SolidLine || m_style == PenStyle::NoPen) {
            return std::vector<double>();
        } else if (m_dashPattern.empty()) {
            const double space = 2;
            const double dot = 1;
            const double dash = 4;
            switch (m_style) {
            case PenStyle::DashLine:
                m_dashPattern = { dash, space };
                break;
            case PenStyle::DotLine:
                m_dashPattern = { dot, space };
                break;
            case PenStyle::DashDotLine:
                m_dashPattern = { dash, space, dot, space };
                break;
            case PenStyle::DashDotDotLine:
                m_dashPattern = { dash, space, dot, space, dot, space };
                break;
            default:
                break;
            }
        }
        return m_dashPattern;
    }

    void setDashPattern(const std::vector<double>& pattern)
    {
        if (pattern.empty()) {
            return;
        }
        m_dashPattern = pattern;
        m_style = PenStyle::CustomDashLine;
        if ((m_dashPattern.size() % 2) == 1) {
            m_dashPattern.push_back(1);
        }
    }

    double widthF() const
    {
        return m_width;
    }

    void setWidthF(double width)
    {
        m_width = width;
    }

    Color color() const
    {
        return m_color;
    }

    void setColor(const Color& color)
    {
        m_color = color;
    }

    PenCapStyle capStyle() const
    {
        return m_capStyle;
    }

    void setCapStyle(PenCapStyle pcs)
    {
        m_capStyle = pcs;
    }

    PenJoinStyle joinStyle() const
    {
        return m_joinStyle;
    }

    void setJoinStyle(PenJoinStyle pcs)
    {
        m_joinStyle = pcs;
    }

#ifndef NO_QT_SUPPORT
    static QPen toQPen(const Pen& pen)
    {
        QPen p(pen.m_color.toQColor(), pen.m_width, static_cast<Qt::PenStyle>(pen.m_style),
               static_cast<Qt::PenCapStyle>(pen.m_capStyle),
               static_cast<Qt::PenJoinStyle>(pen.m_joinStyle));
        p.setDashPattern(QVector<double>(pen.m_dashPattern.cbegin(), pen.m_dashPattern.cend()));
        return p;
    }

    static Pen fromQPen(const QPen& pen)
    {
        Pen p(pen.color(), pen.widthF(), static_cast<PenStyle>(pen.style()),
              static_cast<PenCapStyle>(pen.capStyle()),
              static_cast<PenJoinStyle>(pen.joinStyle()));

        QVector<double> dp = pen.dashPattern();
        p.m_dashPattern = std::vector<double>(dp.cbegin(), dp.cend());
        return p;
    }

#endif

private:

    Color m_color = Color::BLACK;
    double m_width = 1.0;
    PenStyle m_style = PenStyle::SolidLine;
    PenCapStyle m_capStyle = PenCapStyle::SquareCap;
    PenJoinStyle m_joinStyle = PenJoinStyle::BevelJoin;
    mutable std::vector<double> m_dashPattern;
};
} // namespace muse::draw

#endif // MUSE_DRAW_PEN_H
