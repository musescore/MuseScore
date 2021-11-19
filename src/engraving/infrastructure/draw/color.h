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

#ifndef MU_DRAW_COLOR_H
#define MU_DRAW_COLOR_H

#include <QString>

#ifndef NO_QT_SUPPORT
#include <QColor>
#endif

#include "rgba.h"

namespace mu::draw {
class Color
{
public:
    Color();
    Color(const Color& other);
    Color(int red, int green, int blue, int alpha = DEFAULT_ALPHA);
    Color(const char* color);

#ifndef NO_QT_SUPPORT
    Color(const QColor& color);
#endif

    ~Color() = default;

    Color& operator=(const Color& other);
#ifndef NO_QT_SUPPORT
    Color& operator=(const QColor& other);
#endif

    bool operator<(const Color& other) const;

    std::string toString() const;
#ifndef NO_QT_SUPPORT
    QString toQString() const;
#endif

    void setNamedColor(const std::string& color);
    void setNamedColor(const char* color);
    void setRed(int value);
    void setGreen(int value);
    void setBlue(int value);
    void setAlpha(int value);

    bool operator==(const Color& other) const;
    bool operator!=(const Color& other) const;

    bool isValid() const;

    int red() const;
    int green() const;
    int blue() const;
    int alpha() const;

#ifndef NO_QT_SUPPORT
    static Color fromQColor(const QColor& color);
    QColor toQColor() const;
#endif

    static constexpr int DEFAULT_ALPHA = 255;

    static const Color black;
    static const Color white;
    static const Color transparent;
    static const Color redColor;
    static const Color greenColor;
    static const Color blueColor;

private:

    void setRgba(int r, int g, int b, int a);
    void setRgba(Rgba rgb);

    Rgba m_rgba = rgba(0, 0, 0, DEFAULT_ALPHA);
    bool m_isValid = false;
};

inline const Color Color::black { 0, 0, 0 };
inline const Color Color::white { 255, 255, 255 };
inline const Color Color::transparent { 255, 255, 255, 0 };
inline const Color Color::redColor { 255, 0, 0 };
inline const Color Color::greenColor { 0, 255, 0 };
inline const Color Color::blueColor { 0, 0, 255 };
}

#endif // MU_DRAW_COLOR_H
