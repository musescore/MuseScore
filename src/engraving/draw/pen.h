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
#ifndef MU_DRAW_PEN_H
#define MU_DRAW_PEN_H

#include <QColor>
#include "drawtypes.h"
#include "global/log.h"

namespace mu::draw {

class Pen {
public:
    
    Pen(PenStyle style)
    {
        _style = style;
    }
    
    Pen(const QColor &color = Qt::black, double width = 1, PenStyle s = PenStyle::SolidLine, PenCapStyle c = PenCapStyle::SquareCap, PenJoinStyle j = PenJoinStyle::BevelJoin)
        : _color(color), _width(width), _style(s), _capStyle(c), _joinStyle(j)
    {
    }
    
    PenStyle style() const
    {
        return _style;
    }
    
    void setStyle(PenStyle style)
    {
        if (_style == style)
            return;
        _style = style;
        _dashPattern.clear();
      }
    
    std::vector<double> dashPattern() const
    {
        if (_style == PenStyle::SolidLine || _style == PenStyle::NoPen) {
            return std::vector<double>();
        } else if (_dashPattern.empty()) {
            const double space = 2;
            const double dot = 1;
            const double dash = 4;
            switch (_style) {
            case PenStyle::DashLine:
                _dashPattern = {dash, space};
                break;
            case PenStyle::DotLine:
                _dashPattern = {dot, space};
                break;
            case PenStyle::DashDotLine:
                _dashPattern = {dash, space, dot, space};
                 break;
            case PenStyle::DashDotDotLine:
                _dashPattern = {dash, space, dot, space, dot, space};
                break;
            default:
                break;
            }
        }
        return _dashPattern;
    }
    
    void setDashPattern(const std::vector<double> &pattern)
    {
        if (pattern.empty())
            return;
        _dashPattern = pattern;
        _style = PenStyle::CustomDashLine;
        if ((_dashPattern.size() % 2) == 1) {
            LOGW() << "Pattern not of even length";
            _dashPattern.push_back(1);
        }
    }
    
    double widthF() const
    {
        return _width;
    }
    
    void setWidthF(double width)
    {
        _width = width;
    }

    QColor color() const
    {
        return _color;
    }
    
    void setColor(const QColor &color)
    {
        _color = color;
    }
    
    PenCapStyle capStyle() const
    {
        return _capStyle;
    }
    
    void setCapStyle(PenCapStyle pcs)
    {
        _capStyle = pcs;
    }
    
    PenJoinStyle joinStyle() const
    {
        return _joinStyle;
    }
    
    void setJoinStyle(PenJoinStyle pcs)
    {
        _joinStyle = pcs;
    }
    
#ifndef NO_QT_SUPPORT
    static QPen toQPen(const Pen& pen)
    {
        return QPen(pen._color, pen._width, static_cast<Qt::PenStyle>(pen._style), static_cast<Qt::PenCapStyle>(pen._capStyle), static_cast<Qt::PenJoinStyle>(pen._joinStyle));
    }
    static Pen fromQPen(const QPen& pen)
    {
        return Pen(pen.color(), pen.widthF(), static_cast<PenStyle>(pen.style()), static_cast<PenCapStyle>(pen.capStyle()), static_cast<PenJoinStyle>(pen.joinStyle()));
    }
#endif

private:
    
    QColor _color           = Qt::black;
    double _width           = 1;
    PenStyle _style         = PenStyle::SolidLine;
    PenCapStyle _capStyle   = PenCapStyle::SquareCap;
    PenJoinStyle _joinStyle = PenJoinStyle::BevelJoin;
    mutable std::vector<double> _dashPattern;
};

} // namespace mu::draw

#endif // MU_DRAW_PEN_H
