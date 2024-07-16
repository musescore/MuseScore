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

#ifndef MUSE_DRAW_COLOR_H
#define MUSE_DRAW_COLOR_H

#include <string>

#include "global/types/string.h"
#include "global/types/number.h"

#include "rgba.h"

#ifndef NO_QT_SUPPORT
#include <QColor>
#endif

namespace muse::draw {
class Color
{
public:
    Color();
    Color(const Color& other);
    constexpr Color(int r, int g, int b, int a = DEFAULT_ALPHA)
        :  m_rgba(rgba(r, g, b, a)), m_isValid(isRgbaValid(r, g, b, a)) {}

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
    static Color fromString(const std::string& str);
    static Color fromString(const char* str);
    static Color fromString(const String& str);

    void setNamedColor(const std::string& color);
    void setNamedColor(const char* color);
    void setRed(int value);
    void setGreen(int value);
    void setBlue(int value);
    void setAlpha(int value);

    void applyTint(double tint);

    Color inverted() const;

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

    static const Color transparent;
    static const Color BLACK;
    static const Color WHITE;
    static const Color RED;
    static const Color ORANGE;
    static const Color SAFFRON;
    static const Color YELLOW;
    static const Color BRIGHT_YELLOW;
    static const Color LIME_GREEN;
    static const Color GREEN;
    static const Color TEAL;
    static const Color BLUE;
    static const Color INDIGO;
    static const Color VIOLET;
    static const Color MAGENTA;
    static const Color BROWN;
    static const Color GREY;

private:

    void setRgba(int r, int g, int b, int a);
    void setRgba(Rgba rgb);

    Rgba m_rgba = rgba(0, 0, 0, DEFAULT_ALPHA);
    bool m_isValid = false;
};

inline const Color Color::transparent = { 255, 255, 255, 0 };
inline const Color Color::BLACK = { 0, 0, 0, 255 };
inline const Color Color::WHITE = { 255, 255, 255, 255 };
inline const Color Color::RED = { 226, 28, 72, 255 };
inline const Color Color::ORANGE = { 242, 102, 34, 255 };
inline const Color Color::SAFFRON = { 249, 157, 28, 255 };
inline const Color Color::YELLOW = { 255, 204, 51, 255 };
inline const Color Color::BRIGHT_YELLOW = { 255, 243, 43, 255 };
inline const Color Color::LIME_GREEN = { 188, 216, 95, 255 };
inline const Color Color::GREEN = { 98, 188, 71, 255 };
inline const Color Color::TEAL = { 0, 156, 149, 255 };
inline const Color Color::BLUE = { 0, 113, 187, 255 };
inline const Color Color::INDIGO = { 94, 80, 161, 255 };
inline const Color Color::VIOLET = { 141, 91, 166, 255 };
inline const Color Color::MAGENTA = { 207, 62, 150, 255 };
inline const Color Color::BROWN = { 148, 97, 51, 255 };
inline const Color Color::GREY = { 198, 196, 193, 255 };

inline Color blendColors(const Color& c1, const Color& c2)
{
    float alpha1 = c1.alpha() / 255.f;
    float alpha2 = c2.alpha() / 255.f;

    float alphaOut = alpha2 + alpha1 * (1 - alpha2);

    if (is_zero(alphaOut)) {
        return Color(0, 0, 0, 0);
    }
    int r = (c2.red() * alpha2 + c1.red() * alpha1 * (1 - alpha2)) / alphaOut;
    int g = (c2.green() * alpha2 + c1.green() * alpha1 * (1 - alpha2)) / alphaOut;
    int b = (c2.blue() * alpha2 + c1.blue() * alpha1 * (1 - alpha2)) / alphaOut;

    return Color(r, g, b, alphaOut * 255);
}

inline Color blendColors(const Color& c1, const Color& c2, float alpha)
{
    float alpha1 = 1.0;
    float alpha2 = alpha;

    int r = (c2.red() * alpha2 + c1.red() * alpha1 * (1 - alpha2));
    int g = (c2.green() * alpha2 + c1.green() * alpha1 * (1 - alpha2));
    int b = (c2.blue() * alpha2 + c1.blue() * alpha1 * (1 - alpha2));

    return Color(r, g, b, 255);
}

#ifndef NO_QT_SUPPORT
inline QColor blendQColors(const QColor& c1, const QColor& c2)
{
    float alpha1 = c1.alphaF();
    float alpha2 = c2.alphaF();

    float alphaOut = alpha2 + alpha1 * (1 - alpha2);

    if (is_zero(alphaOut)) {
        return QColor(0, 0, 0, 0);
    }
    int r = (c2.red() * alpha2 + c1.red() * alpha1 * (1 - alpha2)) / alphaOut;
    int g = (c2.green() * alpha2 + c1.green() * alpha1 * (1 - alpha2)) / alphaOut;
    int b = (c2.blue() * alpha2 + c1.blue() * alpha1 * (1 - alpha2)) / alphaOut;

    return QColor(r, g, b, alphaOut * 255);
}

inline QColor blendQColors(const QColor& c1, const QColor& c2, float alpha)
{
    float alpha1 = 1.0;
    float alpha2 = alpha;

    int r = (c2.red() * alpha2 + c1.red() * alpha1 * (1 - alpha2));
    int g = (c2.green() * alpha2 + c1.green() * alpha1 * (1 - alpha2));
    int b = (c2.blue() * alpha2 + c1.blue() * alpha1 * (1 - alpha2));

    return QColor(r, g, b, 255);
}

#endif
}

#endif // MUSE_DRAW_COLOR_H
