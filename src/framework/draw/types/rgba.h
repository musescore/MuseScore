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

#ifndef MUSE_DRAW_RGBA_H
#define MUSE_DRAW_RGBA_H

#include <cstdint>

namespace muse::draw {
using Rgba = uint32_t;

// masks RGB values
inline constexpr Rgba RGB_MASK = 0x00ffffff;

// set RGB value
inline constexpr Rgba rgb(int r, int g, int b)
{
    return (0xffu << 24) | ((r & 0xffu) << 16) | ((g & 0xffu) << 8) | (b & 0xffu);
}

// set RGBA value
inline constexpr Rgba rgba(int r, int g, int b, int a)
{
    return ((a & 0xffu) << 24) | ((r & 0xffu) << 16) | ((g & 0xffu) << 8) | (b & 0xffu);
}

// get red part of RGB
inline constexpr int getRed(Rgba rgba)
{
    return (rgba >> 16) & 0xff;
}

// get green part of RGB
inline constexpr int getGreen(Rgba rgba)
{
    return (rgba >> 8) & 0xff;
}

// get blue part of RGB
inline constexpr int getBlue(Rgba rgba)
{
    return rgba & 0xff;
}

// get alpha part of RGBA
inline constexpr int getAlpha(Rgba rgba)
{
    return rgba >> 24;
}

inline constexpr bool isValidComp(int num)
{
    return (num >= 0) && (num < 256);
}

inline constexpr bool isRgbaValid(int r, int g, int b, int a)
{
    return isValidComp(r)
           && isValidComp(g)
           && isValidComp(b)
           && isValidComp(a);
}
}

#endif // MUSE_DRAW_RGBA_H
