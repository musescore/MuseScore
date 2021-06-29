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
    
    Pen() { setDefaults(); }
    
    Pen(PenStyle style) {
        setDefaults();
        _style = style;
    }
    
    Pen(const QColor &color) {
        setDefaults();
        _brush = color;
    }
    
    Pen(const QBrush &brush, double width, PenStyle s = SolidLine,
        PenCapStyle c = SquareCap, PenJoinStyle j = BevelJoin) :
    _brush(brush), _width(width), _style(s), _capStyle(c), _joinStyle(j) {}
    
    PenStyle style() const { return _style; }
    void setStyle(PenStyle style) {
        if (_style == style)
            return;
        _style = style;
        _dashPattern.clear();
      }
    
    QVector<double> dashPattern() const
    {
        if (_style == SolidLine || _style == NoPen) {
            return QVector<qreal>();
        } else if (_dashPattern.isEmpty()) {
            const qreal space = 2;
            const qreal dot = 1;
            const qreal dash = 4;
            switch (_style) {
            case DashLine:
                _dashPattern.reserve(2);
                _dashPattern << dash << space;
                break;
            case DotLine:
                _dashPattern.reserve(2);
                _dashPattern << dot << space;
                break;
            case DashDotLine:
                _dashPattern.reserve(4);
                _dashPattern << dash << space << dot << space;
                break;
            case DashDotDotLine:
                _dashPattern.reserve(6);
                _dashPattern << dash << space << dot << space << dot << space;
                break;
            default:
                break;
            }
        }
        return _dashPattern;
    }
    
    void setDashPattern(const QVector<double> &pattern)
    {
        if (pattern.isEmpty())
            return;
        _dashPattern = pattern;
        _style = CustomDashLine;
        if ((_dashPattern.size() % 2) == 1) {
            LOGW() << "Pattern not of even length";
            _dashPattern << 1;
        }
    }
    
    double widthF() const { return _width; }
    void setWidthF(double width) { _width = width; }


    int width() const { return qRound(_width); }
    void setWidth(int width) { _width = width; }

    QColor color() const { return _brush.color(); }
    void setColor(const QColor &color) { _brush = QBrush(color); }
    QBrush brush() const { return _brush; }
    void setBrush(const QBrush &brush) { _brush = brush; }
    bool isSolid() const { return _brush.style() == Qt::SolidPattern; }
    PenCapStyle capStyle() const { return _capStyle; }
    void setCapStyle(PenCapStyle pcs) { _capStyle = pcs; }
    PenJoinStyle joinStyle() const { return _joinStyle; }
    void setJoinStyle(PenJoinStyle pcs) { _joinStyle = pcs; }
    
#ifndef NO_QT_SUPPORT
    QPen toQPen() const { return QPen(_brush, _width, static_cast<Qt::PenStyle>(_style), static_cast<Qt::PenCapStyle>(_capStyle), static_cast<Qt::PenJoinStyle>(_joinStyle)); }
#endif

private:
    void setDefaults() {
        _brush = Qt::black;
        _width = _defaultWidth;
        _style = _penDefaultStyle;
        _capStyle = _penDefaultCap;
        _joinStyle = _penDefaultJoin;
    }
    
    QBrush _brush;
    double _width;
    PenStyle _style;
    PenCapStyle _capStyle;
    PenJoinStyle _joinStyle;
    mutable QVector<double> _dashPattern; s
    
    static constexpr unsigned int _defaultWidth    = 1;
    static constexpr PenStyle     _penDefaultStyle = SolidLine;
    static constexpr PenCapStyle  _penDefaultCap   = SquareCap;
    static constexpr PenJoinStyle _penDefaultJoin  = BevelJoin;
};

} // namespace mu::draw

#endif // MU_DRAW_PEN_H
