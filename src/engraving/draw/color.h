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

#ifndef NO_QT_SUPPORT
#include <QColor>
#endif

#include "globalcolor.h"
#include "rgb.h"

namespace mu::draw {
class Color
{
public:
    Color() {}
    Color(const Color& other);
    Color(int red, int green, int blue, int alpha = 255);
    Color(GlobalColor color);
    Color(const char* color);

#ifndef NO_QT_SUPPORT
    Color(const QColor& color);
    Color(Qt::GlobalColor color);
#endif

    ~Color() = default;

    Color& operator=(const Color& other);
    Color& operator=(GlobalColor color);
#ifndef NO_QT_SUPPORT
    Color& operator=(const QColor& other);
    Color& operator=(Qt::GlobalColor color);
#endif

    void setNamedColor(const std::string& color) noexcept;
    void setNamedColor(const char* color) noexcept;
    void setRed(int value) noexcept;
    void setGreen(int value) noexcept;
    void setBlue(int value) noexcept;
    void setAlpha(int value) noexcept;

    bool operator==(const Color& other) const noexcept;
    bool operator!=(const Color& other) const noexcept;
    bool operator==(const GlobalColor& color) const noexcept;
    bool operator!=(const GlobalColor& color) const noexcept;

    bool isValid() const noexcept;

    int red() const noexcept;
    int green() const noexcept;
    int blue() const noexcept;
    int alpha() const noexcept;

    std::string name() const noexcept;

#ifndef NO_QT_SUPPORT
    static Color fromQColor(const QColor& color);
    QColor toQColor() const;
#endif

private:

    static constexpr bool isRgbaValid(int r, int g, int b, int a = 255) noexcept
    {
        return uint(r) <= 255 && uint(g) <= 255 && uint(b) <= 255 && uint(a) <= 255;
    }

    void setRgb(int r, int g, int b, int a);
    void setRgb(Rgb rgb);

    Rgb m_rgb = 0;
    std::string m_name;
    bool m_isValid = true;
};
}

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(mu::draw::Color);
#endif

#endif // MU_DRAW_COLOR_H
