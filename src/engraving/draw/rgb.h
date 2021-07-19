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

#ifndef MU_DRAW_RGB_H
#define MU_DRAW_RGB_H

namespace mu::draw {
typedef unsigned int Rgb;

// non-namespaced global variable
inline constexpr Rgb RGB_MASK = 0x00ffffff;     // masks RGB values

inline constexpr Rgb rgb(int r, int g, int b)         // set RGB value
{
    return (0xffu << 24) | ((r & 0xffu) << 16) | ((g & 0xffu) << 8) | (b & 0xffu);
}

inline constexpr Rgb rgba(int r, int g, int b, int a) // set RGBA value
{
    return ((a & 0xffu) << 24) | ((r & 0xffu) << 16) | ((g & 0xffu) << 8) | (b & 0xffu);
}

inline constexpr int getRed(Rgb rgb)                     // get red part of RGB
{
    return (rgb >> 16) & 0xff;
}

inline constexpr int getGreen(Rgb rgb)                   // get green part of RGB
{
    return (rgb >> 8) & 0xff;
}

inline constexpr int getBlue(Rgb rgb)                    // get blue part of RGB
{
    return rgb & 0xff;
}

inline constexpr int getAlpha(Rgb rgb)                   // get alpha part of RGBA
{
    return rgb >> 24;
}
}

#endif // MU_DRAW_RGB_H
