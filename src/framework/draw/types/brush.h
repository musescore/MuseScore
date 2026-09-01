/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#ifndef MUSE_DRAW_BRUSH_H
#define MUSE_DRAW_BRUSH_H

#include <algorithm>
#include <vector>

#include "color.h"
#include "drawtypes.h"
#include "geometry.h"
#include "global/realfn.h"

#ifndef NO_QT_SUPPORT
#include <QBrush>
#include <QGradient>
#include <QLinearGradient>
#endif

namespace muse::draw {
class LinearGradient
{
public:
    struct ColorStop {
        double position = 0.0;
        Color color = Color::BLACK;

        inline bool operator==(const ColorStop& o) const { return RealIsEqual(position, o.position) && color == o.color; }
        inline bool operator!=(const ColorStop& o) const { return !this->operator==(o); }
    };

    LinearGradient() = default;
    LinearGradient(const PointF& start, const PointF& finalStop)
        : m_start(start), m_finalStop(finalStop) {}

    inline const PointF& start() const { return m_start; }
    void setStart(const PointF& start) { m_start = start; }
    inline const PointF& finalStop() const { return m_finalStop; }
    void setFinalStop(const PointF& finalStop) { m_finalStop = finalStop; }

    void setColorAt(double position, const Color& color)
    {
        for (ColorStop& stop : m_colorStops) {
            if (RealIsEqual(stop.position, position)) {
                stop.color = color;
                return;
            }
        }

        auto it = std::lower_bound(m_colorStops.begin(), m_colorStops.end(), position,
                                   [](const ColorStop& stop, double pos) { return stop.position < pos; });
        m_colorStops.insert(it, { position, color });
    }

    inline const std::vector<ColorStop>& colorStops() const { return m_colorStops; }

    inline bool operator==(const LinearGradient& o) const
    {
        return m_start == o.m_start && m_finalStop == o.m_finalStop && m_colorStops == o.m_colorStops;
    }

    inline bool operator!=(const LinearGradient& o) const { return !this->operator==(o); }

private:
    PointF m_start;
    PointF m_finalStop;
    std::vector<ColorStop> m_colorStops;
};

class Brush
{
public:
    Brush() {}
    Brush(BrushStyle style)
        : m_style(style) {}
    Brush(const Color& color)
        : m_color(color) {}
    Brush(const LinearGradient& gradient)
        : m_style(BrushStyle::LinearGradientPattern), m_linearGradient(gradient) {}

#ifndef NO_QT_SUPPORT
    Brush(const QColor& color)
        : m_color(Color::fromQColor(color)) {}
#endif

    inline bool operator==(const Brush& o) const
    {
        if (m_style != o.m_style) {
            return false;
        }

        if (m_style == BrushStyle::LinearGradientPattern) {
            return m_linearGradient == o.m_linearGradient;
        }

        return m_color == o.m_color;
    }

    inline bool operator!=(const Brush& o) const { return !this->operator==(o); }

    inline BrushStyle style() const { return m_style; }
    void setStyle(BrushStyle style) { m_style = style; }
    inline const Color& color() const { return m_color; }
    void setColor(const Color& color) { m_color = color; }
    inline const LinearGradient& linearGradient() const { return m_linearGradient; }
    void setLinearGradient(const LinearGradient& gradient)
    {
        m_style = BrushStyle::LinearGradientPattern;
        m_linearGradient = gradient;
    }

#ifndef NO_QT_SUPPORT
    static QBrush toQBrush(const Brush& brush)
    {
        if (brush.m_style == BrushStyle::LinearGradientPattern) {
            QLinearGradient gradient(brush.m_linearGradient.start().toQPointF(), brush.m_linearGradient.finalStop().toQPointF());
            for (const LinearGradient::ColorStop& stop : brush.m_linearGradient.colorStops()) {
                gradient.setColorAt(stop.position, stop.color.toQColor());
            }
            return QBrush(gradient);
        }

        return QBrush(brush.m_color.toQColor(), static_cast<Qt::BrushStyle>(brush.m_style));
    }

    static Brush fromQBrush(const QBrush& qbrush)
    {
        if (qbrush.style() == Qt::LinearGradientPattern && qbrush.gradient()
            && qbrush.gradient()->type() == QGradient::LinearGradient) {
            const QLinearGradient* qgradient = static_cast<const QLinearGradient*>(qbrush.gradient());
            LinearGradient gradient(PointF::fromQPointF(qgradient->start()), PointF::fromQPointF(qgradient->finalStop()));
            for (const QGradientStop& stop : qgradient->stops()) {
                gradient.setColorAt(stop.first, Color::fromQColor(stop.second));
            }
            return Brush(gradient);
        }

        Brush brush(Color::fromQColor(qbrush.color()));
        brush.setStyle(static_cast<BrushStyle>(qbrush.style()));
        return brush;
    }

#endif

private:
    Color m_color = Color::BLACK;
    BrushStyle m_style = BrushStyle::SolidPattern;
    LinearGradient m_linearGradient;
};
} // namespace muse::draw

#endif // MUSE_DRAW_BRUSH_H
