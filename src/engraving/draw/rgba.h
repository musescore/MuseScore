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

#ifndef MU_DRAW_RGBA_H
#define MU_DRAW_RGBA_H

namespace mu::draw {
using Rgba = unsigned int;

// non-namespaced global variable
inline constexpr Rgba RGB_MASK = 0x00ffffff;     // masks RGB values

inline constexpr Rgba rgb(int r, int g, int b)         // set RGB value
{
    return (0xffu << 24) | ((r & 0xffu) << 16) | ((g & 0xffu) << 8) | (b & 0xffu);
}

inline constexpr Rgba rgba(int r, int g, int b, int a) // set RGBA value
{
    return ((a & 0xffu) << 24) | ((r & 0xffu) << 16) | ((g & 0xffu) << 8) | (b & 0xffu);
}

inline constexpr int getRed(Rgba rgba)                     // get red part of RGB
{
    return (rgba >> 16) & 0xff;
}

inline constexpr int getGreen(Rgba rgba)                   // get green part of RGB
{
    return (rgba >> 8) & 0xff;
}

inline constexpr int getBlue(Rgba rgba)                    // get blue part of RGB
{
    return rgba & 0xff;
}

inline constexpr int getAlpha(Rgba rgba)                   // get alpha part of RGBA
{
    return rgba >> 24;
}
}

#endif // MU_DRAW_RGBA_H
